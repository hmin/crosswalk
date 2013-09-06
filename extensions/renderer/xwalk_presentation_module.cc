// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_presentation_module.h"

#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_view_observer.h"
#include "ipc/ipc_message.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebSecurityOrigin.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_module.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"

using content::RenderView;
using content::RenderViewObserver;
using WebKit::WebDocument;
using WebKit::WebFrame;
using WebKit::WebSecurityOrigin;
using WebKit::WebString;
using WebKit::WebView;

namespace xwalk {
namespace extensions {

namespace {

const char* kExtensionModuleName = "navigator.presentation";

bool IsProtocolAllowed(const WebString& protocol) {
  if (protocol == "file")
    return true;
  return false;
}

}  // namespace

class PresentationMessageListener : public RenderViewObserver {
 public:
  PresentationMessageListener(RenderView* render_view);
  virtual ~PresentationMessageListener();

  static void DispatchMessage(RenderView* render_view, const base::Value& msg);

 private:
  // RenderViewObserver implementations.
  virtual void OnDestruct() OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  void OnShowPresentationSucceeded(int request_id, int view_id);
  void OnShowPresentationFailed(int request_id,
                                const std::string& error_message);

  v8::Handle<v8::Context> v8_context_;
};

PresentationMessageListener::PresentationMessageListener(RenderView* view)
  : RenderViewObserver(view) {
}

PresentationMessageListener::~PresentationMessageListener() {
}

void PresentationMessageListener::OnDestruct() {
}

bool PresentationMessageListener::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(PresentationMessageListener, message)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_ShowPresentationSucceeded,
                        OnShowPresentationSucceeded)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_ShowPresentationFailed,
                        OnShowPresentationFailed)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void PresentationMessageListener::OnShowPresentationSucceeded(
    int request_id, int view_id) {
//  base::ListValue msg;
//  msg.AppendString("THIS is a message");
  base::DictionaryValue dict;
  dict.SetString("cmd", "ShowSucceed");
  dict.SetInteger("request_id", request_id);
  dict.SetInteger("view_id", view_id);
  DispatchMessage(render_view(), dict);
}

void PresentationMessageListener::OnShowPresentationFailed(
    int request_id, const std::string& message) {
  base::DictionaryValue dict;
  dict.SetString("cmd", "ShowFailed");
  dict.SetInteger("request_id", request_id);
  dict.SetString("error_message", message);
  DispatchMessage(render_view(), dict);
}

// static
void PresentationMessageListener::DispatchMessage(RenderView* render_view,
    const base::Value& message) {
  WebFrame* frame = render_view->GetWebView()->mainFrame();

  DCHECK(frame);

  v8::HandleScope handle_scope;
  v8::Handle<v8::Context> context = frame->mainWorldScriptContext();
  XWalkModuleSystem* module_system =
      XWalkModuleSystem::GetModuleSystemFromContext(context);
  CHECK(module_system);

  XWalkExtensionModule* extension_module =
      module_system->GetExtensionModule(kExtensionModuleName);

  extension_module->DispatchMessageToListener(context, message);
}

XWalkPresentationModule::XWalkPresentationModule() {
}

XWalkPresentationModule::~XWalkPresentationModule() {
  if (!object_template_.IsEmpty()) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    object_template_.Dispose(isolate);
    object_template_.Clear();
  }
}

// static
RenderView* XWalkPresentationModule::GetCurrentRenderView() {
  WebFrame* frame = WebFrame::frameForCurrentContext();
  DCHECK(frame) << "There should be an active frame here";

  if (!frame)
    return NULL;

  WebView* view = frame->view();
  if (!view)
    return NULL;

  return RenderView::FromWebView(view);
}

// static
void XWalkPresentationModule::GetViewIDFromCurrentContext(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  RenderView* render_view = XWalkPresentationModule::GetCurrentRenderView();

  if (!render_view) {
    info.GetReturnValue().Set(v8::Integer::New(-1));
    return;
  }

  info.GetReturnValue().Set(v8::Integer::New(render_view->GetRoutingID()));
}

// static
void XWalkPresentationModule::GetWindowContext(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() != 1)
    return;

  if (!args[0]->IsInt32())
    return;

  int new_view_id = args[0]->Int32Value();
  if (new_view_id == MSG_ROUTING_NONE)
    return;

  RenderView* cur_view = GetCurrentRenderView();
  RenderView* new_view = RenderView::FromRoutingID(new_view_id);
  if (!new_view)
    return;

  WebFrame* opener = cur_view->GetWebView()->mainFrame();
  WebFrame* frame = new_view->GetWebView()->mainFrame();
  frame->setOpener(opener);
  v8::Local<v8::Value> window = frame->mainWorldScriptContext()->Global();
  args.GetReturnValue().Set(window);
}

void XWalkPresentationModule::RequestShowPresentation(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() != 3)
    return;
  if (!args[0]->IsInt32() || !args[1]->IsInt32() ||
      (!args[2]->IsStringObject() && !args[2]->IsString()))
    return;

  int request_id = args[0]->Int32Value();
  int opener_id = args[1]->Int32Value();
  std::string target_url = *v8::String::Utf8Value(args[2]->ToString());

  DLOG(INFO) << "request id: " << request_id
             << " opener id: " << opener_id
             << " target url: " << target_url;

  RenderView* render_view = XWalkPresentationModule::GetCurrentRenderView();
  if (!render_view)
    return;

  // Since the success callback of navigator.requestShow accepts a WindowProxy
  // object, which means the script access to the presentation browsing context
  // is require, at least window.postMessage can work. However, there is no
  // way for cross-process script access. As a result, we have to prevent
  // showing a cross-origin url as presentation context.
  GURL url(target_url);
  WebDocument document = WebFrame::frameForCurrentContext()->document();
  WebSecurityOrigin opener = document.securityOrigin();
  WebSecurityOrigin target = WebSecurityOrigin::create(url);

  // Only allow the same origin or allowed protocol (e.g. file://).
  if (opener.canRequest(url) || (opener.protocol() == target.protocol() &&
      IsProtocolAllowed(opener.protocol()))) {
    render_view->Send(
        new XWalkViewHostMsg_RequestShowPresentation(render_view->GetRoutingID(),
                                                     request_id,
                                                     opener_id,
                                                     target_url));
    return;
  }

  // Otherwise, notify the opener a security error was occurred.
  base::DictionaryValue dict;
  dict.SetString("cmd", "ShowFailed");
  dict.SetInteger("request_id", request_id);
  dict.SetString("error_message", "SecurityError");

  PresentationMessageListener::DispatchMessage(render_view, dict);
}

v8::Handle<v8::Object> XWalkPresentationModule::NewInstance() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  if (object_template_.IsEmpty()) {
    v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
    templ->Set("GetViewID",
               v8::FunctionTemplate::New(
                   XWalkPresentationModule::GetViewIDFromCurrentContext));
    templ->Set("RequestShowPresentation",
               v8::FunctionTemplate::New(
                   XWalkPresentationModule::RequestShowPresentation));
    templ->Set("GetWindowContext",
               v8::FunctionTemplate::New(
                   XWalkPresentationModule::GetWindowContext));
    object_template_.Reset(isolate, templ);
    message_listener_.reset(
        new PresentationMessageListener(GetCurrentRenderView()));
  }

  v8::Handle<v8::ObjectTemplate> object_template =
      v8::Handle<v8::ObjectTemplate>::New(isolate, object_template_);
  return handle_scope.Close(object_template->NewInstance());
}

}  // namespace extensions
}  // namespace xwalk
