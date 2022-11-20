export function blobToBase64(blob) {
    try {
        let reader = new FileReader();
        reader.readAsDataURL(blob);
        return new Promise((resolve) => {
            reader.onloadend = () => {
                resolve(reader.result);
            };
        });
    } catch (err) {
        console.log('error: ', err);
        return new Promise((resolve) => null);
    }
}