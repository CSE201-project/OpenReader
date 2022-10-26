const fetch = require('node-fetch');

class DummySolariumDevice {
  IsConnected = false;
  constructor(host) {
    this.setHost(host);
  }

  setHost(IP) {
    this.Host = IP.replace(/^\s+|\s+$/g, '');
    this._urlBase = 'http://' + this.Host + ':8080';
  }

  async _fetchWithTimeout(uri, options) {
    const { timeout = 8000 } = options;
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), timeout);

    const response = await fetch(uri, {
      ...options,
      signal: controller.signal
    });

    clearTimeout(timeoutId);

    return response;
  }

  async info() {
    let uri = `${this._urlBase}/info`;
    try {
      let response = await this._fetchWithTimeout(uri, {
        method: 'GET',
        mode: 'cors',
        headers: { 'Content-Type': 'application/json' },
        timeout: 3000
      });
      if (response.status === 200) {
        let infoResult = await response.json();
        return (infoResult);
      }
    } catch (error) {
      console.debug(error);
    }
    return null;
  }

  async connect() {
    let info = await this.info();
    if (info) {
      this.Name = info.name;
      this.IsConnected = true;
      this.solariumVersion = info.version;
      return true;
    } else {
      return false;
    }
  }
}

describe('Connection', function () {

  describe('Connect to device with no IP', function () {
    it('Should abort connection', async function (done) {
      let device = new DummySolariumDevice('');
      let success = await device.connect();
      expect(success).toEqual(false);
      done();
    });
  });
  describe('Connect to device with invalid IP', function () {
    it('Should abort connection', async function (done) {
      let device = new DummySolariumDevice('257.0.0.0');
      let success = await device.connect();
      expect(success).toEqual(false);
      done();
    });
  });
  describe('Connect to device with valid IP', function () {
    it('Should connect successfully', async function (done) {
      let device = new DummySolariumDevice('localhost');
      let success = await device.connect();
      expect(success).toEqual(true);
      done();
    });
  });
});