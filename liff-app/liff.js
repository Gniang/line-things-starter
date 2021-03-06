// User service UUID: Change this to your generated service UUID
const USER_SERVICE_UUID = '9DD76B74-9301-4904-9D22-9096F561F4F8'; // LED, Button

// User service characteristics
const USE1_CHARACTERISTIC_UUID = 'E9062E71-9E62-4BC6-B0D3-35CDCD9B027B';

const RTN1_CHARACTERISTIC_UUID = '62FBD229-6EDD-4D1A-B554-5C4E1BB29169';


// PSDI Service UUID: Fixed value for Developer Trial
const PSDI_SERVICE_UUID = 'E625601E-9E55-4597-A598-76018A0D293D'; // Device ID
const PSDI_CHARACTERISTIC_UUID = '26E2B12B-85F0-4F3F-9FDD-91D114270E6E';

// UI settings
let clickCount = 0;

// -------------- //
// On window load //
// -------------- //

window.onload = () => {
    initializeApp();
};

// ----------------- //
// Handler functions //
// ----------------- //

function handlerUseBtnOnClick(value) {
    liffSendNum(value);
    sendRireki(value);
}


function handlerRefresh() {
    liffSendNum("0");
}

function appendLog(message) {
    const el = document.getElementById("logs");
    var log = document.createElement('span');
    var br = document.createElement('br');
    log.innerText = message;

    //最後の子要素として追加
    el.appendChild(log);
    el.appendChild(br);
}

// ------------ //
// UI functions //
// ------------ //

function uiToggleUseButton(state, btn_name) {
    const el = document.getElementById(btn_name);
    console.log(el);
    el.innerText = state ? "USED" : "FREE";
    if (state) {
        el.classList.remove("led-on");
    } else {
        el.classList.add("led-on");
    }
}

function uiCountPressButton() {
    clickCount++;

    const el = document.getElementById("click-count");
    el.innerText = clickCount;
}


function uiToggleDeviceConnected(connected) {
    const elStatus = document.getElementById("status");
    const elControls = document.getElementById("controls");

    elStatus.classList.remove("error");

    if (connected) {
        // Hide loading animation
        uiToggleLoadingAnimation(false);
        // Show status connected
        elStatus.classList.remove("inactive");
        elStatus.classList.add("success");
        elStatus.innerText = "Device connected";
        // Show controls
        elControls.classList.remove("hidden");
    } else {
        // Show loading animation
        uiToggleLoadingAnimation(true);
        // Show status disconnected
        elStatus.classList.remove("success");
        elStatus.classList.add("inactive");
        elStatus.innerText = "Device disconnected";
        // Hide controls
        elControls.classList.add("hidden");
    }
}

function uiToggleLoadingAnimation(isLoading) {
    const elLoading = document.getElementById("loading-animation");

    if (isLoading) {
        // Show loading animation
        elLoading.classList.remove("hidden");
    } else {
        // Hide loading animation
        elLoading.classList.add("hidden");
    }
}

function uiStatusError(message, showLoadingAnimation) {
    uiToggleLoadingAnimation(showLoadingAnimation);

    const elStatus = document.getElementById("status");
    const elControls = document.getElementById("controls");

    // Show status error
    elStatus.classList.remove("success");
    elStatus.classList.remove("inactive");
    elStatus.classList.add("error");
    //elStatus.innerText = message;

    // Hide controls
    elControls.classList.add("hidden");
}

function makeErrorMsg(errorObj) {
    return "Error\n" + errorObj.code + "\n" + errorObj.message;
}

// -------------- //
// LIFF functions //
// -------------- //

function initializeApp() {
    liff.init(() => initializeLiff(), error => uiStatusError(makeErrorMsg(error), false));
}

function initializeLiff() {
    liff.initPlugins(['bluetooth']).then(() => {
        liffCheckAvailablityAndDo(() => liffRequestDevice());
    }).catch(error => {
        uiStatusError(makeErrorMsg(error), false);
    });
}

function liffCheckAvailablityAndDo(callbackIfAvailable) {
    // Check Bluetooth availability
    liff.bluetooth.getAvailability().then(isAvailable => {
        if (isAvailable) {
            uiToggleDeviceConnected(false);
            callbackIfAvailable();
        } else {
            uiStatusError("Bluetooth not available", true);
            setTimeout(() => liffCheckAvailablityAndDo(callbackIfAvailable), 10000);
        }
    }).catch(error => {
        uiStatusError(makeErrorMsg(error), false);
    });;
}

function liffRequestDevice() {
    liff.bluetooth.requestDevice().then(device => {
        liffConnectToDevice(device);
    }).catch(error => {
        uiStatusError(makeErrorMsg(error), false);
    });
}

function liffConnectToDevice(device) {
    device.gatt.connect().then(() => {
        document.getElementById("device-name").innerText = device.name;
        document.getElementById("device-id").innerText = device.id;

        // Show status connected
        uiToggleDeviceConnected(true);

        // Get service
        device.gatt.getPrimaryService(USER_SERVICE_UUID).then(service => {
            liffGetUserService(service);
        }).catch(error => {
            uiStatusError("test:" + makeErrorMsg(error), false);
        });
        device.gatt.getPrimaryService(PSDI_SERVICE_UUID).then(service => {
            liffGetPSDIService(service);
        }).catch(error => {
            uiStatusError(makeErrorMsg(error), false);
        });

        // Device disconnect callback
        const disconnectCallback = () => {
            // Show status disconnected
            uiToggleDeviceConnected(false);

            // Remove disconnect callback
            device.removeEventListener('gattserverdisconnected', disconnectCallback);

            // Reset LED state
            useState1 = false;
            // Reset UI elements
            uiToggleLedButton(false);
            uiToggleStateButton(false);

            // Try to reconnect
            initializeLiff();
        };

        device.addEventListener('gattserverdisconnected', disconnectCallback);
        handlerRefresh();
    }).catch(error => {
        uiStatusError(makeErrorMsg(error), false);
    });
}

function liffGetUserService(service) {
    // Button pressed state
    service.getCharacteristic(RTN1_CHARACTERISTIC_UUID).then(characteristic => {
        liffGetButtonStateCharacteristic(characteristic);
    }).catch(error => {
        uiStatusError(makeErrorMsg(error), false);
    });


    // Toggle LED
    service.getCharacteristic(USE1_CHARACTERISTIC_UUID).then(characteristic => {
        window.ledCharacteristic = characteristic;

        // Switch off by default
        liffSendNum(false);
    }).catch(error => {
        uiStatusError(makeErrorMsg(error), false);
    });
}

function liffGetPSDIService(service) {
    // Get PSDI value
    service.getCharacteristic(PSDI_CHARACTERISTIC_UUID).then(characteristic => {
        return characteristic.readValue();
    }).then(value => {
        // Byte array to hex string
        const psdi = new Uint8Array(value.buffer)
            .reduce((output, byte) => output + ("0" + byte.toString(16)).slice(-2), "");
        document.getElementById("device-psdi").innerText = psdi;
    }).catch(error => {
        uiStatusError(makeErrorMsg(error), false);
    });
}

function liffGetButtonStateCharacteristic(characteristic) {
    // Add notification hook for button state
    // (Get notified when button state changes)
    characteristic.startNotifications().
        then(() => {
            characteristic.addEventListener('characteristicvaluechanged', e => {
                const buff = new Uint8Array(e.target.value.buffer);
                const val = new TextDecoder().decode(buff);
                appendLog(val);
                valueChanged(val);
            });
        }).catch(error => {
            uiStatusError(makeErrorMsg(error), false);
        });
}



function sendRireki(usb_id) {
    $.get('https://us-central1-forestiot.cloudfunctions.net/function-1',
        {
            user: 'developer' + PSDI_SERVICE_UUID, //ToDo デバイスID
            usb_id: usb_id
        })
}


function sendStatus(sts1, sts2, sts3) {
    $.get('https://us-central1-forestiot.cloudfunctions.net/syncUSBRentalStatus',
        {
            usb1: sts1 ? "USED" : "FREE",
            usb2: sts2 ? "USED" : "FREE",
            usb3: sts3 ? "USED" : "FREE",
        })
}

function valueChanged(recieveString) {
    const values = recieveString.split(',');
    const header = values[0];
    const body = values[1];
    const stamp = values[2];
    if (header == "1") {
        const status = ('000' + body).slice(-3);
        const sts1 = status[0] == "1";
        const sts2 = status[1] == "1";
        const sts3 = status[2] == "1";
        uiToggleUseButton(sts1, 'btn_use1');
        uiToggleUseButton(sts2, 'btn_use2');
        uiToggleUseButton(sts3, 'btn_use3');
        console.log(body);
        sendStatus(sts1, sts2, sts3);
    } else if (header == "2") {
        // ToDo status cloud send

    }
}

function setUseButtonState(isUse, btnName) {
    if (isUse) {
        // press
        uiToggleStateButton(true, btnName);
    } else {
        // release
        uiToggleStateButton(false, btnName);
        uiCountPressButton();
    }

}

function liffSendNum(num) {
    appendLog(num);
    console.log("send: " + num);
    const arrayBuffe = new TextEncoder().encode(num);
    window.ledCharacteristic.writeValue(
        arrayBuffe
    ).catch(error => {
        uiStatusError(makeErrorMsg(error), false);
    });
}
