var baseURL = 'https://graph.api.smartthings.com';
var clientID = '93299720-c3bc-436c-96bd-f0a7fdf1bc82';
var redirectURI = 'http://shawnconroyd.com/pebble/smartthings-callback/test.php';

var config;
var options;

var index = 0;

Pebble.addEventListener('ready', function() {
    console.log("Ready");
    
    if(localStorage.getItem("pebble-smartthings-config")) {
        options = JSON.parse(localStorage.getItem("pebble-smartthings-config"));
    }
});

Pebble.addEventListener("showConfiguration", function() {
    console.log("Showing Configuration");
    config = localStorage.getItem("pebble-smartthings-config");
    Pebble.openURL(baseURL + '/oauth/authorize?response_type=code&client_id=' + clientID + '&scope=app&redirect_uri=' + redirectURI);
    //Pebble.openURL('http://shawnconroyd.com/pebble/smartthings-callback/test.php');
});

Pebble.addEventListener("webviewclosed", function(e) {
    console.log("Configuration Closed");
    // webview closed
    options = JSON.parse(decodeURIComponent(e.response));
    console.log("Options = " + JSON.stringify(options));
    localStorage.setItem("pebble-smartthings-config", JSON.stringify(options));
});

function sendNextMessage(messages, length, type) {    
    if (messages.length === 0) {
        index = 0;
        return;
    }
    var json;
    var message = messages.shift();
    
    //console.log("Length = " + length);
    
    if(type == "modes") {
        json = {"MODE_NAME": message, "MODE_INDEX": index, "MODE_FINAL": length};
    }
    else if(type == "switches") {
        json = {"SWITCH_ID": message, "SWITCH_NAME": messages.shift(), "SWITCH_VALUE": messages.shift(), "SWITCH_INDEX": index, "SWITCH_FINAL": length};
        //console.log(JSON.stringify(json));
    }
    else if(type == "locks") {
        json = {"LOCK_ID": message, "LOCK_NAME": messages.shift(), "LOCK_VALUE": messages.shift(), "LOCK_INDEX": index, "LOCK_FINAL": length};
    }
    
    Pebble.sendAppMessage(json,
                           function(e) {
                               console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
                               index = index + 1;
                               sendNextMessage(messages, length, type);
                           },
                           function(e) {
                               console.log("Unable to deliver message with transactionId=" + e.data.transactionId + " Error is: " + e.error.message);
                               messages.unshift();
                               if(type != "modes") {
                                   messages.unshift();
                                   messages.unshift();
                               }
                               sendNextMessage(messages, length, type);
                           }
                          );
}

function fetchModes() {
    console.log("Fetching Modes");
    var response;
    var req = new XMLHttpRequest();
    
    //console.log("Modes URL: " + baseURL + options.endpointURL + "/modes");
    
    req.open('GET', baseURL + options.endpointURL + "/modes", true);
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    req.onload = function(e) {
        if (req.readyState == 4 && req.status == 200) {
            response = JSON.parse(req.responseText);
            sendNextMessage(response, response.length, "modes");
        }
    };
    req.send(null);
}

function setMode(mode) {
    console.log("Setting Mode: " + mode);
    //var response;
    var req = new XMLHttpRequest();
    
    req.open('PUT', baseURL + options.endpointURL + "/modes");
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    req.setRequestHeader('Content-Type', 'application/json');
    //req.onload = function(e) {
        //console.log(JSON.stringify(req.responseText));
    //};
    req.send(JSON.stringify({'mode': mode}));
}

function fetchDevices(type) {
    console.log("Fetching " + type);
    var response;
    var req = new XMLHttpRequest();
    var devices = [];
    
    req.open('Get', baseURL + options.endpointURL + "/" + type , true);
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    //req.setRequestHeader('Accept-Encoding', 'gzip');
    req.onload = function(e) {
        console.log("Ready State: " + req.readyState + " --- Status: " + req.status);
        if(req.readyState == 4 && req.status == 200) {
            response = JSON.parse(req.responseText);
            //console.log(JSON.stringify(response));
            for(var i=0; i < response.length; i++) {
                console.log("ID: " + response[i].id + " - Name: " + response[i].label);
                devices.push(response[i].id);
                devices.push(response[i].label);
                if(response[i].value === null || response[i].value === "") {
                    devices.push("N/A");
                }
                else {
                    devices.push(response[i].value);
                }
            }
            sendNextMessage(devices, response.length, type);
        }
    };
    req.send(null);
}

function updateDevice(id, type) {
    console.log("Toggling Switch");
    var req = new XMLHttpRequest();
    
    req.open('PUT', baseURL + options.endpointURL + "/" + type + "/" + id);
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    req.setRequestHeader('Content-Type', 'application/json');
    req.send(JSON.stringify({'command': "toggle"}));
    
    //setTimeout(getValue(id), 1000);
}

Pebble.addEventListener("appmessage", function(e) {
    console.log("App Message Received");
    
    if(e.payload.FETCH_MODES) {
        fetchModes();
    }
    if(e.payload.MODE_SET) {
        setMode(e.payload.MODE_SET);
    }
    if(e.payload.FETCH_SWITCHES) {
        fetchDevices(e.payload.FETCH_SWITCHES);
    }
    if(e.payload.SWITCH_UPDATE) {
        updateDevice(e.payload.SWITCH_UPDATE, "switches");
    }
    if(e.payload.FETCH_LOCKS) {
        fetchDevices(e.payload.FETCH_LOCKS);
    }
    if(e.payload.LOCK_UPDATE) {
        updateDevice(e.payload.LOCK_UPDATE, "locks");
    }
    if(e.payload.FETCH_MSWITCHES) {
        fetchDevices(e.payload.FETCH_MSWITCHES);
        //fetchMSwitches(e.payload.FETCH_MSWITCHES);
    }
    if(e.payload.MSWITCH_UPDATE) {
        updateDevice(e.payload.MSWITCH_UPDATE, "momentary");
    }
});