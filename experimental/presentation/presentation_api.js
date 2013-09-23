/* Copyright (c) 2013 Intel Corporation. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

var v8toolsNative = requireNative("v8tools");
var internal = requireNative("internal");
internal.setupInternalExtension(extension);

var DISPLAY_AVAILABLE_CHANGE_EVENT = "displayavailablechange";
var _listeners = {}; /* Listeners of display available change event */
var _displayAvailable = false;
var _nextRequestId = 0;
var _showRequests = {};

function DOMError(msg) {
  this.message = msg;
}

function ShowRequest(id, successCallback, errorCallback) {
	this._requestId = id;
	this._successCallback = successCallback;
	this._errorCallback = errorCallback;
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

function requestShowPresentation(url, successCallback, errorCallback) {
	var requestId = ++_nextRequestId;
	var request = new ShowRequest(requestId, successCallback, errorCallback);
	_showRequests[requestId] = request;

  var message = { "cmd": "RequestShow", "requestId": requestId, "url": url };
  extension.postMessage(JSON.stringify(message));
}

extension.setMessageListener(function(json) {
  var msg = JSON.parse(json);
  if (msg.cmd == "DisplayAvailableChange") {
    setTimeout(function() {
      handleDisplayAvailableChange(msg.data /* available? */);
    }, 0);
  } else if (msg.cmd == "ShowSucceeded") {
    setTimeout(function() {
      handleShowSucceeded(msg.requestId, parseInt(msg.data) /* view id */);
    }, 0);
  } else if (msg.cmd == "ShowFailed") {
    setTimeout(function() {
      handleShowFailed(msg.requestId, msg.data /* error message */);
    }, 0);
  } else {
    console.error("Invalid message : " + msg.cmd);
  }
})

function handleDisplayAvailableChange(isAvailable) {
	if (_displayAvailable != isAvailable) {
		_displayAvailable = isAvailable;
		if (!_listeners[DISPLAY_AVAILABLE_CHANGE_EVENT])
			return;
    var length = _listeners[DISPLAY_AVAILABLE_CHANGE_EVENT].length
		for (var i = 0; i < length; ++i) {
			_listeners[DISPLAY_AVAILABLE_CHANGE_EVENT][i].apply(null, null);
		}
	}
}

function handleShowSucceeded(requestId, viewId) {
	var request = _showRequests[requestId];
	if (request) {
    var view = v8toolsNative.GetWindowObject(viewId);
    request._successCallback.apply(null, [view]);
		delete _showRequests[requestId];
	} else {
		console.error("Invalid request id: " + requestId);
	}
}

function handleShowFailed(requestId, errorMessage) {
  var request = _showRequests[requestId];
  if (request) {
    var error = new DOMError(errorMessage);
    request._errorCallback.apply(null, [error]);
    delete _showRequests[requestId];
  } else {
		console.error("Invalid request id." + requestId);
  }
}

exports.requestShow = requestShowPresentation;
exports.addEventListener = addEventListener;
exports.removeEventListener = removeEventListener;
exports.__defineSetter__("on" + DISPLAY_AVAILABLE_CHANGE_EVENT,
  function(callback) {
	  if (callback)
	    addEventListener(DISPLAY_AVAILABLE_CHANGE_EVENT, callback);
	  else
	    removeEventListener(DISPLAY_AVAILABLE_CHANGE_EVENT,
                          this.ondisplayavailablechange);
  }
);

exports.__defineGetter__("displayAvailable", function() {
  /* If there is at least one listener registered to listen the display available
     change, we can rely on the _displayAvailable flag. Otherwise, we need to
     send a sync message to query the availability flag from browser process
     each time. */
  if (!_listeners[DISPLAY_AVAILABLE_CHANGE_EVENT] ||
      _listeners[DISPLAY_AVAILABLE_CHANGE_EVENT].length == 0) {
    var res = extension.internal.sendSyncMessage("QueryDisplayAvailability");
    _displayAvailable = res == "true" ? true : false;
  }
  return _displayAvailable;
});
