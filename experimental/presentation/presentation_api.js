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

function DOMError(name) {
  this.name = name;
}

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

exports.requestShow = function(url, successCallback, errorCallback) {
	var request_id = _next_request_id_++;
  var opener_id = presentationNative.GetViewID();
	var request = new ShowRequest(request_id, successCallback, errorCallback);
	_show_requests[request_id]= request;

  presentationNative.RequestShowPresentation(request_id, opener_id, url);
}

function handleDisplayAvailableChange(is_available) {
	if (exports.displayAvailable != is_available) {
		exports.displayAvailable = is_available;
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
    console.log("view id" + view_id);
    var view = presentationNative.GetWindowContext(view_id);
    request._success_callback.apply(null, [view]);
		delete _show_requests[request_id];
	} else {
		console.log("Unknow error: invalid request id." + request_id);
	}

}

function handleShowFailed(request_id, error_message) {
  var request = _show_requests[request_id];
  if (request) {
    var error = new DOMError(error_message);
    request._error_callback.apply(null, [error]);
    delete _show_requests[request_id];
  } else {
		console.log("Unknow error: invalid request id." + request_id);
  }
}


extension.setMessageListener(function(msg) {
  if (msg.cmd == "DisplayAvailableChange") {
    handleDisplayAvailableChange(msg.data /* available? */);
  } else if (msg.cmd == "ShowSucceed") {
    setTimeout(function() {
      handleShowSucceed(msg.request_id, parseInt(msg.data) /* view id */);
    }, 0);
  } else if (msg.cmd == "ShowFailed") {
    setTimeout(function() {
      handleShowFailed(msg.request_id, msg.data /* error message */);
    }, 0);
  } else {
    console.error("Invalid response message : " + msg.cmd);
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
