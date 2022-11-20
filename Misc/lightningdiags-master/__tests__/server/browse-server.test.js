const chai = require("chai");
const chaiHttp = require("chai-http");
chai.use(chaiHttp);
const expect = require("chai").expect;
const request = require("request");

describe("Browse Screen Server Test", function () {

    var url = "http://localhost:8080/browse";
    var filePath = require('../assets/patients_list.json');
    var patientList;

    it("Should be patient_list", function(done) {
        request(url, function(error, response, body) {
            try {
                patientList = JSON.parse(body);
            } catch (error) {
                console.log('error: not a valid JSON');
            }
            expect(patientList).to.deep.equal(filePath);
            done();
        });
    });
    it('Should be an [Array]', function(done) {
        request(url, function(error, response, body) {
            expect(patientList).to.be.an('Array');
            done();
        });
    });
    it('Should be an {Object} with "id"', function(done) {
        let json = 0;
        request(url, function(error, response, body) {
            while (patientList[json]) {
                expect(patientList[json]).to.be.an('Object');
                expect(patientList[json++]).to.have.property('id');
            }
        done();
        });
    });
});