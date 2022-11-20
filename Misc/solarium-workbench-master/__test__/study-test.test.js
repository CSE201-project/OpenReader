import Study from '../src/Study.js';

describe('Study', function () {
  describe('Add Snapshot', function () {

    it('Should return a valid Snapshot', function (done) {
      let study = new Study("Study1", "0.23\n0.22\n0.21", "./test/path/", "ngml");
      let snapshot = study.addSnapshot("0.23 ngml");

      expect(snapshot.concentration).toEqual("0.23 ngml");
      expect(snapshot.photoNumber).toEqual(1);
      expect(snapshot.fileName).toEqual("./test/path/Study1_0.23ngml_N01.png");
      done();
    });
    it ('Should manage blank correctly', function (done) {
      let study = new Study("Study   2", "0.42\n0.43\n0.44", "./test/path/", "pgml");
      let snapshot = study.addSnapshot("0.42 pgml");

      expect(snapshot.concentration).not.toEqual('0.42pgml');
      expect(snapshot.fileName).toEqual('./test/path/Study   2_0.42pgml_N01.png');
      done();
    });
  });
});