export function currentDate(date) {
    let today = date ? date : new Date();
    let hour = String(today.getHours()).padStart(2, '0');
    let minute = String(today.getMinutes()).padStart(2, '0');
    let second = String(today.getSeconds()).padStart(2, '0');
    let day = String(today.getDate()).padStart(2, '0');
    let month = String(today.getMonth() + 1).padStart(2, '0');
    let year = today.getFullYear();
    today = year + month + day + '-' + hour + minute + second;
    return today;
}