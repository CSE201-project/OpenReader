import { currentDate } from '../src/Tools/CurrentDate';
import { getParamsToSave } from '../src/Tools/SaveParams';

const infoJson = require('../../solarium-dummy-server/files/device_info.json');
const DEFAULT_CAMERA_PARAMS = require('../src/Assets/default-camera-params.json');

class DummySolariumDevice {
  async info() {
    return infoJson;
  }
}

describe('Tools functions', function () {
  const date = new Date('November 23, 2020 03:24:01');
  const device = new DummySolariumDevice();

  describe('currentDate()', function () {
    it('Should be well formated', function (done) {
      let res = currentDate(date);
      expect(res).toEqual("20201123-032401");
      done();
    });
  });
  describe('getParamsToSave()', function () {
    it('Should build object properly', async function (done) {
      let res = await getParamsToSave(device, JSON.stringify(DEFAULT_CAMERA_PARAMS));
      let paramsObj = JSON.parse(res);
      expect(paramsObj.name).toEqual('Solarium');
      expect(paramsObj.camera_params).toMatchObject(DEFAULT_CAMERA_PARAMS);
      done();
    });
  });
});