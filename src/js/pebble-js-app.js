var baseURL = 'https://graph.api.smartthings.com';
var clientID = '93299720-c3bc-436c-96bd-f0a7fdf1bc82';
var redirectURI = 'http://shawnconroyd.com/pebble/smartthings-callback/test.php';

var config;
var options;

var index = 0;

var modes;
var switches;
var locks;
var momentary;
//var contact;
//var motion;
var sensors;

Pebble.addEventListener('ready', function() {
    console.log("Ready");
    
    if(localStorage.getItem("pebble-smartthings-config")) {
        options = JSON.parse(localStorage.getItem("pebble-smartthings-config"));
        fetchDevices();
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
    fetchDevices();
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
    else if(type == "momentary") {
        json = {"MSWITCH_ID": message, "MSWITCH_NAME": messages.shift(), "MSWITCH_VALUE": messages.shift(), "MSWITCH_INDEX": index, "MSWITCH_FINAL": length};
    }
    else if(type == "sensors") {
        json = {"SENSOR_ID": message, "SENSOR_NAME": messages.shift(), "SENSOR_VALUE": messages.shift(), "SENSOR_TYPE": messages.shift(), "SENSOR_INDEX": index, "SENSOR_FINAL": length};
    }
    /*
    else if(type == "contact") {
        json = {"CONTACT_ID":message, "CONTACT_NAME": messages.shift(), "CONTACT_VALUE": messages.shift(), "CONTACT_INDEX": index, "CONTACT_FINAL": length};
    }
    else if(type == "motion") {
        json = {"MOTION_ID": message, "MOTION_NAME": messages.shift(), "MOTION_VALUE": messages.shift(), "MOTION_INDEX": index, "MOTION_FINAL": length};
    }
    */
    
    Pebble.sendAppMessage(json,
                           function(e) {
                               console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
                               index = index + 1;
                               sendNextMessage(messages, length, type);
                           },
                           function(e) {
                               console.log("Unable to deliver message with transactionId=" + e.data.transactionId + " Error is: " + e.error.message);
                               messages.unshift();
                               if(type != "modes" || type != "sensors") {
                                   messages.unshift();
                                   messages.unshift();
                               }
                               else if(type == "sensors") {
                                   messages.unshift();
                                   messages.unshift();
                                   messages.unshift();
                               }
                               sendNextMessage(messages, length, type);
                           }
                          );
}

function fetchModes() {
    console.log("Fetching Modes");
    var req = new XMLHttpRequest();
    
    //console.log("Modes URL: " + baseURL + options.endpointURL + "/modes");
    
    req.open('GET', baseURL + options.endpointURL + "/modes", true);
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    req.onload = function(e) {
        if (req.readyState == 4 && req.status == 200) {
            modes = JSON.parse(req.responseText);
            //sendNextMessage(response, response.length, "modes");
        }
    };
    req.send(null);
}

function fetchSwitches(refresh) {
    console.log("Fetching Switches");
    switches = [];
    var response;
    var req = new XMLHttpRequest();
    
    req.open('GET', baseURL + options.endpointURL + "/switches", true);
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    req.onload = function(e) {
        if(req.readyState == 4 && req.status == 200) {
            response = JSON.parse(req.responseText);
            for(var i=0; i < response.length; i++) {
                switches.push(response[i].id);
                switches.push(response[i].label);
                switches.push(response[i].value);
            }
        }
        if(refresh) {
            sendNextMessage(switches, switches.length/3, "switches");
        }
    };
    req.send(null);
}

function fetchLocks(refresh) {
    console.log("Fetching Locks");
    locks = [];
    var response;
    var req = new XMLHttpRequest();
    
    req.open('GET', baseURL + options.endpointURL + "/locks", true);
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    req.onload = function(e) {
        if(req.readyState == 4 && req.status == 200) {
            response = JSON.parse(req.responseText);
            for(var i=0; i < response.length; i++) {
                locks.push(response[i].id);
                locks.push(response[i].label);
                locks.push(response[i].value);
            }
        }
        if(refresh) {
            sendNextMessage(locks, locks.length/3, "locks");
        }
    };
    req.send(null);
}

function fetchMomentary(refresh) {
    console.log("Fetching Momentary Switches");
    momentary = [];
    var response;
    var req = new XMLHttpRequest();
    
    req.open('GET', baseURL + options.endpointURL + "/momentary", true);
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    req.onload = function(e) {
        if(req.readyState == 4 && req.status == 200) {
            response = JSON.parse(req.responseText);
            for(var i=0; i < response.length; i++) {
                momentary.push(response[i].id);
                momentary.push(response[i].label);
                momentary.push("N/A");
            }
        }
        if(refresh) {
            sendNextMessage(momentary, momentary.length/3, "momentary");
        }
    };
    req.send(null);
}

function fetchSensors() {
    sensors = [];
    fetchContact();
    fetchMotion();
}

function fetchContact() {
    console.log("Fetching Contact Sensors");
    //contact = [];
    var response;
    var req = new XMLHttpRequest();
    
    req.open('GET', baseURL + options.endpointURL + "/contactSensors", true);
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    req.onload = function() {
        if(req.readyState == 4 && req.status == 200) {
            response = JSON.parse(req.responseText);
            for(var i=0; i < response.length; i++) {
                //console.log("Name: " + response[i].label + " --- Value: " + response[i].value);
                /*
                if(response[i].id == options.sensorID) {
                    var momentaryID = momentary.indexOf(options.momentaryID);
                    momentary[momentaryID+2] = response[i].value;
                }
                else {
                */
                sensors.push(response[i].id);
                sensors.push(response[i].label);
                sensors.push(response[i].value);
                sensors.push("contactSensor");
                //}
            }
        }
    };
    req.send(null);
}

function fetchMotion() {
    console.log("Fetching Motion Sensors");
    //motion = [];
    var response;
    var req = new XMLHttpRequest();
    
    req.open('GET', baseURL + options.endpointURL + "/motionSensors", true);
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    req.onload = function() {
        if(req.readyState == 4 && req.status == 200) {
            response = JSON.parse(req.responseText);
            for(var i=0; i < response.length; i++) {
                //console.log("Name: " + response[i].label + " --- Value: " + response[i].value);
                sensors.push(response[i].id);
                sensors.push(response[i].label);
                sensors.push(response[i].value);
                sensors.push("motionSensor");
            }
        }
    };
    req.send(null);
}

function fetchDevices() {
    fetchModes();
    fetchSwitches(false);
    fetchLocks(false);
    fetchMomentary(false);
    fetchSensors();
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

function updateDevice(id, type) {
    console.log("Toggling Switch");
    var req = new XMLHttpRequest();
    
    req.open('PUT', baseURL + options.endpointURL + "/" + type + "/" + id);
    req.setRequestHeader('Authorization', 'Bearer ' + options.accessToken);
    req.setRequestHeader('Content-Type', 'application/json');
    req.send(JSON.stringify({'command': "toggle"}));
}

Pebble.addEventListener("appmessage", function(e) {
    console.log("App Message Received");
    
    if(e.payload.FETCH_MODES) {
        //fetchModes();
        sendNextMessage(modes, modes.length, "modes");
    }
    if(e.payload.MODE_SET) {
        setMode(e.payload.MODE_SET);
    }
    if(e.payload.FETCH_SWITCHES) {
        //fetchDevices(e.payload.FETCH_SWITCHES);
        sendNextMessage(switches, switches.length/3, "switches");
    }
    if(e.payload.SWITCH_UPDATE) {
        updateDevice(e.payload.SWITCH_UPDATE, "switches");
        setTimeout(function(){fetchSwitches(true);}, 750);
    }
    if(e.payload.FETCH_LOCKS) {
        //fetchDevices(e.payload.FETCH_LOCKS);
        sendNextMessage(locks, locks.length/3, "locks");
    }
    if(e.payload.LOCK_UPDATE) {
        updateDevice(e.payload.LOCK_UPDATE, "locks");
    }
    if(e.payload.FETCH_MSWITCHES) {
        //fetchDevices(e.payload.FETCH_MSWITCHES);
        //fetchMSwitches(e.payload.FETCH_MSWITCHES);
        sendNextMessage(momentary, momentary.length/3, "momentary");
    }
    if(e.payload.MSWITCH_UPDATE) {
        updateDevice(e.payload.MSWITCH_UPDATE, "momentary");
    }
    if(e.payload.FETCH_SENSORS) {
        sendNextMessage(sensors, sensors.length/4, "sensors");
    }
});