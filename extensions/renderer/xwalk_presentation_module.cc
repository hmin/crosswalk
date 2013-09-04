// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_presentation_module.h"

#include "content/public/renderer/render_view.h"
#include "ipc/ipc_message.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"

using content::RenderView;
using WebKit::WebFrame;
using WebKit::WebView;

namespace xwalk {
namespace extensions {

namespace {

RenderView* GetCurrentRenderView() {
  WebFrame* frame = WebFrame::frameForCurrentContext();
  DCHECK(frame) << "There should be an active frame here";

  if (!frame)
    return NULL;

  WebView* view = frame->view();
  if (!view)
    return NULL;

  return RenderView::FromWebView(view);
}

void GetRoutingIDFromCurrentContext(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  RenderView* render_view = GetCurrentRenderView();

  if (!render_view) {
    info.GetReturnValue().Set(v8::Integer::New(-1));
    return;
  }

  info.GetReturnValue().Set(v8::Integer::New(render_view->GetRoutingID()));
}

void GetWindowContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() != 1)
    return;

  if (!args[0]->IsInt32())
    return;

  int new_view_id = args[0]->Int32Value();
  if (new_view_id == MSG_ROUTING_NONE)
    return;

  RenderView* new_view = RenderView::FromRoutingID(new_view_id);
  if (!new_view)
    return;

  WebFrame* frame = new_view->GetWebView()->mainFrame();
  v8::Local<v8::Value> window = frame->mainWorldScriptContext()->Global();
  args.GetReturnValue().Set(window);
}

void RequestShowPresentation(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() != 3)
    return;
  if (!args[0]->IsInt32() || !args[1]->IsInt32() ||
      (!args[2]->IsStringObject() && !args[2]->IsString()))
    return;
  int request_id = args[0]->Int32Value();
  int opener_id = args[1]->Int32Value();
  std::string target_url = *v8::String::Utf8Value(args[2]->ToString());

  RenderView* render_view = GetCurrentRenderView();
  if (!render_view)
    return;

  int routing_id = render_view->GetRoutingID();
  render_view->Send(
      new XWalkViewHostMsg_RequestShowPresentation(routing_id,
                                                   request_id,
                                                   opener_id,
                                                   target_url));
  return;
}

}  // namespace

XWalkPresentationModule::XWalkPresentationModule() {
}

XWalkPresentationModule::~XWalkPresentationModule() {
  if (!object_template_.IsEmpty()) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    object_template_.Dispose(isolate);
    object_template_.Clear();
  }
}

v8::Handle<v8::Object> XWalkPresentationModule::NewInstance() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  if (object_template_.IsEmpty()) {
    v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
    templ->Set("GetRoutingID",
               v8::FunctionTemplate::New(GetRoutingIDFromCurrentContext));
    templ->Set("RequestShowPresentation",
               v8::FunctionTemplate::New(RequestShowPresentation));
    object_template_.Reset(isolate, templ);
  }

  v8::Handle<v8::ObjectTemplate> object_template =
      v8::Handle<v8::ObjectTemplate>::New(isolate, object_template_);
  return handle_scope.Close(object_template->NewInstance());
}

}  // namespace extensions
}  // namespace xwalk
