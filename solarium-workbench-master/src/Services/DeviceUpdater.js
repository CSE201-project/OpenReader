'use strict'

const SOLARIUM_FILEPATH = '/opt/Solarium/bin/Solarium';
const SOLARIUM_ROOTPATH = '/opt/Solarium';
const SOLARIUM_RC_SCRIPT = '/etc/init.d/solarium.sh';

async function getRemoteHostName(activeSshConnection)
{
    let res = await activeSshConnection.execCommand(`hostname`);
    return res.stdout;
}

async function isDeviceSeries3(activeSshConnection)
{
    let hostname = await getRemoteHostName(activeSshConnection);
    return (hostname.match(/^lmx-3/));
}

async function startRemoteSolarium(activeSshConnection, allServices)
{
    if (await isDeviceSeries3(activeSshConnection))
    {
        await activeSshConnection.execCommand("systemctl start solarium");
        if (allServices) {
            await activeSshConnection.execCommand("systemctl start lmx3-leds");
        }
    }
    else
    {
        const data = [];
        const shell = await activeSshConnection.requestShell();
        shell.on('data', function(chunk) {
            data.push(chunk);
        })
        shell.write(`${SOLARIUM_RC_SCRIPT} start \n`);    
        await new Promise(r => setTimeout(r, 3000));
        shell.end();
    }
    await new Promise(r => setTimeout(r, 3000));
}

async function stopRemoteSolarium(activeSshConnection, allServices)
{
    if (await isDeviceSeries3(activeSshConnection))
    {
        await activeSshConnection.execCommand("systemctl stop solarium");
        if (allServices) {
            await activeSshConnection.execCommand("systemctl stop lmx3-leds");
        }
    }
    else
    {
        await activeSshConnection.execCommand(`${SOLARIUM_RC_SCRIPT} stop`, {
            cwd: '/home/root', 
            execOptions: {pty:true}
        });

        await activeSshConnection.exec('killall', ['-q', '-9', 'Solarium'], {
            cwd: '/',
            onStdout(chunk) {
                console.log('stdoutChunk', chunk.toString('utf8'))
            },
            onStderr(chunk) {
                console.log('stderrChunk', chunk.toString('utf8'))
            }
        });

        await new Promise(r => setTimeout(r, 3000));
    }
}

async function pushSolariumOnly(nodeSsh, filePath, deviceHost, progressCb) {

    const ssh = new nodeSsh();
    const sshConfig = {
        host: deviceHost,
        username: 'root'
    };

    try {
        console.log(`==> Connecting to = ${deviceHost} to upload file ${filePath}`);
        await ssh.connect(sshConfig);
        await ssh.putFile(filePath, SOLARIUM_FILEPATH + '.new', null, {
            step: (total_transferred, chunk, total) => {
                let uploadProgress = Math.floor(total_transferred / total * 100);

                if (progressCb) {
                    progressCb( Math.floor(uploadProgress * 0.80) );
                }
            }
        });
        
        let res = await ssh.execCommand(`chmod +x ${SOLARIUM_FILEPATH}.new`);
        
        await stopRemoteSolarium(ssh);
        if (progressCb) {
            await progressCb( 90 );
        }

        await ssh.execCommand(`mv ${SOLARIUM_FILEPATH} ${SOLARIUM_FILEPATH}.old`);
        await ssh.execCommand(`mv ${SOLARIUM_FILEPATH}.new ${SOLARIUM_FILEPATH}`);
        
        await startRemoteSolarium(ssh);
        if (progressCb) {
            await progressCb( 100 );
        }

        return ({
            success: true,
            message: 'Everything went fine'
        });
    }
    catch (err) {

        return ({
            success: false,
            message: 'Something went wrong:' + err
        });
    }

}


async function pushSolariumZip(nodeSsh, filePath, deviceHost, progressCb) {

    const ssh = new nodeSsh();
    const sshConfig = {
        host: deviceHost,
        username: 'root'
    };

    try {
        console.log(`==> Connecting to = ${deviceHost} to upload file ${filePath}`);
        await ssh.connect(sshConfig);
        await ssh.putFile(filePath, SOLARIUM_ROOTPATH + '/solarium.zip', null, {
            step: (total_transferred, chunk, total) => {
                let uploadProgress = Math.floor(total_transferred / total * 100);

                if (progressCb) {
                    progressCb( Math.floor(uploadProgress * 0.80) );
                }
            }
        });
        
        await stopRemoteSolarium(ssh, true);

        if (progressCb) {
            progressCb( 90 );
        }

        await ssh.execCommand(`cp -a ${SOLARIUM_ROOTPATH}/bin/Solarium.conf ${SOLARIUM_ROOTPATH}`);
        await ssh.execCommand(`rm -rf ${SOLARIUM_ROOTPATH}/bin ${SOLARIUM_ROOTPATH}/lib`);
        await ssh.execCommand(`unzip ${SOLARIUM_ROOTPATH}/solarium.zip -d ${SOLARIUM_ROOTPATH}`);
        await ssh.execCommand(`cp -a ${SOLARIUM_ROOTPATH}/Solarium.conf ${SOLARIUM_ROOTPATH}/bin/Solarium.conf`);
        
        if (progressCb) {
            progressCb( 95 );          
        }

        await ssh.execCommand(`ldconfig`);
        await startRemoteSolarium(ssh, true);

        if (progressCb) {
            progressCb( 100 );
        }

        return ({
            success: true,
            message: 'Everything went fine'
        });
    }
    catch (err) {

        return ({
            success: false,
            message: 'Something went wrong:' + err
        });
    }
}


export async function pushFileToDevice(nodeSsh, filePath, deviceHost, progressCb) {

    if (filePath.endsWith(".zip")) {
        return await pushSolariumZip(nodeSsh, filePath, deviceHost, progressCb);
    } else {
        return await pushSolariumOnly(nodeSsh, filePath, deviceHost, progressCb);
    }

}
