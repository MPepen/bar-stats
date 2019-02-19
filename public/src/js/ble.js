// These 128-Bit ID's correspond to the Nordic Semi-conductor 'UART' BLE service which is used by Adafruit and others.
const UART_SERVICE_UUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const UART_CHAR_RX_UUID = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';
const UART_CHAR_TX_UUID = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';

const SENSOR_SERVICE_UUID = 0xB500;
const ACCEL_CHAR_UUID = 0xB501;
const MAG_CHAR_UUID = 0xB502
const GYRO_CHAR_UUID = 0xB503;

const delay = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

class BLEHandler {
    /**
     * 
     * @param {string} name 
     */
    constructor() {
        this.device = undefined;
        this.server = undefined;
        this.accelChar = undefined;
        this.magChar = undefined;
        this.gyroChar = undefined;
    }

    connect(name) {

        const onNotificationEvent = (event) => {
            if (event.target.value.byteLength > 0) {
                console.log(event.target.value);
                const data = new Uint8Array(event.target.value.buffer);
                console.info("recv data: " + data);
            }
        };
        return navigator.bluetooth.requestDevice({
            filters: [
                {namePrefix: name},
            ],
            optionalServices: [UART_SERVICE_UUID], // changed to use only UART service
        })
        .then((d) => {
            console.info('connected to device: ', d);
            this.device=d;
            return this.device.gatt.connect();
        })
        .then((srv) => {
            console.info('connected to gatt server: ', srv);
            this.server = srv;
            return this.server.getPrimaryService(UART_SERVICE_UUID);// changed to use only UART service
        })
        .then((svc) => {
            console.info('got service: ', svc);
            this.service = svc;
            return this.service.getCharacteristic(UART_CHAR_RX_UUID);// changed to only use RX services
        })
        .then((c) => {
            console.info('got accel characteristic: ', c);
            this.accelChar = c;
            this.accelChar.addEventListener('characteristicvaluechanged', onNotificationEvent);
            return this.service.getCharacteristic(UART_CHAR_RX_UUID);// changed to only use RX services
        })
        .then((c) => {
            console.info('got mag characteristic: ', c);
            this.magChar = c;
            this.magChar.addEventListener('characteristicvaluechanged', onNotificationEvent);
            return this.service.getCharacteristic(UART_CHAR_RX_UUID);// changed to only use RX services
        })
        .then((c) => {
            console.info('got characteristic: ', c);
            this.gyroChar = c;
            this.gyroChar.addEventListener('characteristicvaluechanged', onNotificationEvent);
            console.info('all characteristics found, attached all event listeners');
        })
        .catch((e) => {
            console.error("error connecting ble: ", e)
            console.error(e.stack)
        });
    }

    startNotifications() {
        this.accelChar.startNotifications()
            .catch((e) => {
                console.error('failed to start accel notifications: ', e);
                console.error(e.stack);
            });
        this.magChar.startNotifications()
            .catch((e) => {
                console.error('failed to start mag notifications: ', e);
                console.error(e.stack);
            });
        this.gyroChar.startNotifications()
            .catch((e) => {
                console.error('failed to start gyro notifications: ', e);
                console.error(e.stack);
            });
    }

    stopNotifications() {
        this.accelChar.stopNotifications()
            .catch((e) => {
                console.error('failed to stop accel notifications: ', e);
                console.error(e.stack);
            });
        this.magChar.stopNotifications()
            .catch((e) => {
                console.error('failed to stop mag notifications: ', e);
                console.error(e.stack);
            });
        this.gyroChar.stopNotifications()
            .catch((e) => {
                console.error('failed to stop gyro notifications: ', e);
                console.error(e.stack);
            });
    }
}
