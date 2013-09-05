// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var presentationNative = requireNative("presentationNative");

extension._setupExtensionInternal();
var internal = extension._internal;

var _next_request_id_ = 0;
var _show_requests = {};
var _listeners = {}; // Listeners of display available change event
var DISPLAY_AVAILABLE_CHANGE_EVENT = "displayavailablechange";

function ShowRequest(id, successCallback, errorCallback) {
	this._request_id = id;
	this._success_callback = successCallback;
	this._error_callback = errorCallback;
}

function addEventListener(name, callback, useCapture /* ignored */) {
  if (!_listeners[name])
  	_listeners[name] = [];
  _listeners[name].push(callback)
}

function removeEventListener(name, callback) {
  if (_listeners[name]) {
  	var index = _listeners[name].indexOf(callback);
  	if (index != -1)
	  _listeners[name].splice(index, 1);
  }
}

function handleDisplayAvailableChange(msg) {
	if (exports.displayAvailable != msg.displayAvailable) {
		exports.displayAvailable = msg.displayAvailable;
		if (!_listeners[DISPLAY_AVAILABLE_CHANGE_EVENT])
			return;
		for (var i = 0; i < _listeners[DISPLAY_AVAILABLE_CHANGE_EVENT].length; ++i) {
			_listeners[DISPLAY_AVAILABLE_CHANGE_EVENT][i].apply(null, null);
		}
	}
}

function handleShowSucceed(request_id, view_id) {
	var request = _show_requests[request_id];
	if (request) {
    // Request to show presentation succeed.
    // Get window object from view_id
    console.log("### view id" + view_id);
    var view = presentationNative.GetWindowContext(view_id);
    request._success_callback.apply(null, [view]);
    console.log("no error happened");
		delete _show_requests[request_id];
	} else {
		// TODO(hmin): throw an error.
		console.log("Invalid request id: " + request_id);
	}

}

exports.requestShow = function(url, successCallback, errorCallback) {
	var request_id = _next_request_id_++;
  var opener_id = presentationNative.GetViewID();
	var request = new ShowRequest(request_id, successCallback, errorCallback);
	_show_requests[request_id]= request;

  console.log("start a request to show presentation");
  presentationNative.RequestShowPresentation(request_id, opener_id, url);
//	extension._internal.postMessage("requestShow", [request_id, opener_id, url]);
}

extension.setMessageListener(function(msg) {
  if (msg.cmd == "DisplayAvailableChange") {
    handleDisplayAvailableChange(msg);
  } else if (msg.cmd == "ShowSucceed") {
    setTimeout(function() {
      handleShowSucceed(msg.request_id, msg.view_id);
    }, 0);
//    handleShowSucceed(msg);
  } else {
    console.error("Invalid response : " + msg.cmd);
  }
})

exports.displayAvailable = false;
exports.addEventListener = addEventListener;
exports.removeEventListener = removeEventListener;
exports.__defineSetter__("on"+DISPLAY_AVAILABLE_CHANGE_EVENT, function(callback) {
	if (callback)
	  addEventListener(DISPLAY_AVAILABLE_CHANGE_EVENT, callback);
	else
	  removeEventListener(DISPLAY_AVAILABLE_CHANGE_EVENT, this.ondisplayavailablechange);
})
