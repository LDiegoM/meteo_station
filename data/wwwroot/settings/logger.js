function saveLogger() {
    const body = getUpdateBody();

    executeCall("/settings/logger", "PUT", body).then((updOk) => {
        if (updOk == true) {
            document.location.reload(true);
        }
    });
}

function getUpdateBody() {
    const writePeriod = document.getElementById("writePeriod").value;
    var body = '{"write_period":' + writePeriod + '}';
    return body;
}
