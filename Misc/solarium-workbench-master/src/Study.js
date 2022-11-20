export default class Study {
    constructor(studyName, concentrationListString, pathToSave, concentrationUnit) {
        this.name = studyName;        
        this.path = pathToSave;
        this.unit = concentrationUnit;
        this.concentrationList = this._createConcentrationList(concentrationListString, concentrationUnit);
    }

    _createConcentrationList(concentrationListString, concentrationUnit)
    {
        let concentrationList = {};
        let list = concentrationListString.split('\n');    
        for (let i = 0; list[i]; i++) {
            let concentrationName = list[i] + ' ' + concentrationUnit;
            concentrationList[concentrationName] = [];
        }
        return concentrationList;
    }

    addConcentration(concentrationListString, concentrationUnit) {
        let concentrationList = this._createConcentrationList(concentrationListString, concentrationUnit);
        this.concentrationList = Object.assign(this.concentrationList, concentrationList);
    }

    addSnapshot(concentrationName) {
        let snapshot = {};
        
        snapshot.concentration = concentrationName;
        snapshot.photoNumber = Object.keys(this.concentrationList[concentrationName]).length + 1;
        snapshot.fileName = `${this.path}/${this.name}/${this.name}_${concentrationName.replace(" ", "")}_N${String(snapshot.photoNumber).padStart(2, '0')}.png`;

        this.concentrationList[concentrationName].push(snapshot);
        return snapshot;
    }
}