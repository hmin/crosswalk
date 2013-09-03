// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// var presentationInternal = requireNative("presentation");

var _next_request_id_ = 0;
var _show_requests = {};
var _listeners = {}; // Listeners of display available change event
var DISPLAY_AVAILABLE_CHANGE_EVENT = "displayavailablechange";

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

function handleShowRequest(msg) {
  var request_id = msg.request_id;
	var request = _show_requests[msg.request_id];
	if (request) {
		if (msg.error_code == 0) {
			// Request to show presentation succeed.
			var view_id = msg.view_id;
			// Get window object from view_id
			// var w = presentativeNative.GetView(view_id)
			// request.success_callback.apply(null, [w]);
		} else {
			// Create DOMError from error code.
			// var e = createDOMError(msg.error_code);
			// request.error_callback.apply(null, [e])
		}
		delete _show_requests[request_id];
	} else {
		// TODO(hmin): throw an error.
		console.log("Invalid request id in navigator.presentation calling.")
	}

}

function ShowRequest(id, successCallback, errorCallback) {
	this._request_id = id;
	this._success_callback = successCallback;
	this._error_callback = errorCallback;
}

exports.requestShow = function(url, successCallback, errorCallback) {
	var request_id = _next_request_id_++;
	var request = new ShowRequest(request_id, successCallback, errorCallback);
	_requests[request_id]= request;
	var msg = {
		"cmd" : "RequestShow",
		"url" : url,
		"request_id" : request_id
	};
	extension.postMessage(JSON.stringify(msg));
}

extension.setMessageListener(function(json) {
	var msg = JSON.parse(json);
	if (msg.cmd == "DisplayAvailableChange") {
		handleDisplayAvailableChange(msg);
	} else {
		handleShowRequest(msg);
	}
})

exports.displayAvailable = false;
exports.addEventListener = addEventListener;
exports.removeEventListener = removeEventListener;
exports.__defineSetter__("on" + DISPLAY_AVAILABLE_CHANGE_EVENT, function(callback) {
	if (callback)
	  addEventListener(DISPLAY_AVAILABLE_CHANGE_EVENT, callback);
	else
	  removeEventListener(DISPLAY_AVAILABLE_CHANGE_EVENT, this.ondisplayavailablechange);
})