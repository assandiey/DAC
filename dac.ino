#include <WiFi.h>
#include <WebServer.h>
#include <math.h>

#define RXD2 16
#define TXD2 17

const char* ssid = "EMB7010_PROJET_DAC";
const char* password = " ";

WebServer serveur(80);

static String ligneUart;
static float derniereTension_V = NAN;
static const uint32_t FENETRE_MS = 20000;

const char page_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>DAQ Miniature - Visualisation Web</title>
  <style>
    body{font-family:"Times New Roman",Times,serif;background:#fff;color:#000;margin:0;padding:0}
    header{background:#fff;padding:18px 16px;border-bottom:1px solid #ddd;text-align:center}
    .header-lines{line-height:1.35}
    .l1{font-size:34px;font-weight:700}
    .l2{font-size:20px;font-weight:700;margin-top:6px}
    .l3{font-size:18px;font-weight:700;margin-top:4px}
    .l4,.l5,.l6{font-size:17px;margin-top:4px}
    .l4{margin-top:10px}
    .container{padding:12px 16px}
    .stats{background:#fff;border:1px solid #ddd;padding:10px 12px;margin-bottom:12px;font-size:16px}
    .stats b{display:inline-block;min-width:140px;font-weight:700}
    canvas{background:#fff;border:1px solid #ccc;width:100%;max-width:900px;height:420px}
  </style>
</head>
<body>
  <header>
    <div class="header-lines">
      <div class="l1">UQAM</div>
      <div class="l2">EMB7010 - Construction de logiciel en environnement embarqué</div>
      <div class="l3">Projet - Système d'acquisition de données (DAQ) miniature avec visualisation web</div>
      <div class="l4">Professeur : <b>M. Zakaria El Alaoui Ismaili</b></div>
      <div class="l5">Réaliser par  :  <b>Assane Dieye</b></div>
      <div class="l6">Automne 2025</div>
    </div>
  </header>

  <div class="container">
    <div class="stats">
      <div><b>Dernière valeur :</b> <span id="derniereValeur">---</span> V</div>
      <div><b>Valeur max :</b> <span id="valeurMax">---</span> V</div>
      <div><b>Valeur min :</b> <span id="valeurMin">---</span> V</div>
    </div>

    <canvas id="graphe" width="900" height="420"></canvas>
  </div>

<script>
(() => {
  const canvas = document.getElementById('graphe');
  const ctx = canvas.getContext('2d');

  const Y_MIN = -5.0;
  const Y_MAX =  5.0;
  const FENETRE_MS = 20000;

  let valeurs = [];
  let temps = [];
  let dernierTemps = -1;

  function borner(v, mn, mx){ return Math.max(mn, Math.min(mx, v)); }

  function majStats(){
    const elMin = document.getElementById('valeurMin');
    const elMax = document.getElementById('valeurMax');
    if (valeurs.length === 0){
      elMin.textContent = '---';
      elMax.textContent = '---';
      return;
    }
    let mn = valeurs[0], mx = valeurs[0];
    for (let i = 1; i < valeurs.length; i++){
      if (valeurs[i] < mn) mn = valeurs[i];
      if (valeurs[i] > mx) mx = valeurs[i];
    }
    elMin.textContent = mn.toFixed(3);
    elMax.textContent = mx.toFixed(3);
  }

  function viderCourbe(){
    valeurs = [];
    temps = [];
    dernierTemps = -1;
    document.getElementById('derniereValeur').textContent = '---';
    majStats();
  }

  function dessiner(){
    ctx.fillStyle = '#fff';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    const mg = 78, md = 15, mh = 18, mb = 55;
    const W = canvas.width - mg - md;
    const H = canvas.height - mh - mb;

    const x0 = mg, y0 = mh;
    const x1 = mg + W, y1 = mh + H;

    function yFromV(v){
      const vv = borner(v, Y_MIN, Y_MAX);
      const n = (vv - Y_MIN) / (Y_MAX - Y_MIN);
      return y0 + H * (1 - n);
    }
    function xFromSec(s){
      return x0 + (W * s) / 20.0;
    }

    ctx.strokeStyle = '#e0e0e0';
    ctx.lineWidth = 1;
    for (let v = Y_MIN; v <= Y_MAX + 1e-6; v += 1.0){
      const yy = yFromV(v);
      ctx.beginPath(); ctx.moveTo(x0, yy); ctx.lineTo(x1, yy); ctx.stroke();
    }

    ctx.strokeStyle = '#eaeaea';
    for (let s = 0; s <= 20; s += 1){
      const xx = xFromSec(s);
      ctx.beginPath(); ctx.moveTo(xx, y0); ctx.lineTo(xx, y1); ctx.stroke();
    }

    ctx.strokeStyle = '#888';
    ctx.lineWidth = 1.2;
    ctx.beginPath();
    ctx.moveTo(x0, y0);
    ctx.lineTo(x0, y1);
    ctx.lineTo(x1, y1);
    ctx.stroke();

    ctx.strokeStyle = '#bbb';
    ctx.lineWidth = 1;
    const yZero = yFromV(0);
    ctx.beginPath(); ctx.moveTo(x0, yZero); ctx.lineTo(x1, yZero); ctx.stroke();

    ctx.fillStyle = '#000';
    ctx.font = '14px "Times New Roman", Times, serif';
    ctx.fillText('Temps (s)', x0 + W/2 - 35, y1 + 45);

    ctx.save();
    ctx.translate(26, y0 + H/2 + 35);
    ctx.rotate(-Math.PI/2);
    ctx.fillText('Tension (V)', 0, 0);
    ctx.restore();

    ctx.textAlign = 'right';
    ctx.textBaseline = 'middle';
    const xEtiq = x0 - 8;
    ctx.fillText('5 V',  xEtiq, yFromV(5));
    ctx.fillText('0 V',  xEtiq, yFromV(0));
    ctx.fillText('-5 V', xEtiq, yFromV(-5));
    ctx.textAlign = 'start';
    ctx.textBaseline = 'alphabetic';

    if (valeurs.length < 2){
      ctx.fillStyle = '#000';
      ctx.font = '16px "Times New Roman", Times, serif';
      ctx.fillText("En attente de données UART...", x0 + 10, y0 + 24);
      return;
    }

    const tFin = temps[temps.length - 1];
    const tDebut = tFin - FENETRE_MS;

    ctx.strokeStyle = '#0077cc';
    ctx.lineWidth = 2;
    ctx.beginPath();

    for (let i = 0; i < valeurs.length; i++){
      const sec = (temps[i] - tDebut) / 1000.0;
      const xx = xFromSec(sec);
      const yy = yFromV(valeurs[i]);
      if (i === 0) ctx.moveTo(xx, yy);
      else ctx.lineTo(xx, yy);
    }
    ctx.stroke();
  }

  async function boucle(){
    try{
      const rep = await fetch('/donnee');
      if (!rep.ok) return;
      const d = await rep.json();

      const v = d.value;
      const t = d.time;

      if (typeof v === 'number' && !isNaN(v) && typeof t === 'number'){
        if (dernierTemps >= 0 && t < dernierTemps) viderCourbe();
        dernierTemps = t;

        document.getElementById('derniereValeur').textContent = v.toFixed(3);

        valeurs.push(v);
        temps.push(t);

        const limite = t - FENETRE_MS;
        while (temps.length && temps[0] < limite){
          temps.shift();
          valeurs.shift();
        }

        majStats();
        dessiner();
      }
    }catch(e){}
  }

  majStats();
  dessiner();
  setInterval(boucle, 500);
})();
</script>
</body>
</html>
)rawliteral";

static bool extraireTension(const String &s, float *outV)
{
  String x = s;
  x.trim();
  if (x.length() == 0) return false;

  int debut = -1;
  for (int i = 0; i < (int)x.length(); i++){
    char c = x[i];
    if ((c >= '0' && c <= '9') || c == '-') { debut = i; break; }
  }
  if (debut < 0) return false;

  int fin = debut;
  while (fin < (int)x.length()){
    char c = x[fin];
    if (!((c >= '0' && c <= '9') || c == '.' || c == '-')) break;
    fin++;
  }

  float brut = x.substring(debut, fin).toFloat();
  if (!isfinite(brut)) return false;
  if (fabsf(brut) > 20.0f) brut /= 1000.0f;

  *outV = brut;
  return true;
}

static void page()
{
  serveur.send(200, "text/html", page_html);
}

static void donnee()
{
  String json = "{";
  json += "\"value\":";
  if (isnan(derniereTension_V)) json += "null";
  else json += String(derniereTension_V, 4);
  json += ",\"time\":";
  json += String(millis());
  json += "}";
  serveur.send(200, "application/json", json);
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  serveur.on("/", page);
  serveur.on("/donnee", donnee);
  serveur.begin();
}

void loop()
{
  serveur.handleClient();

  while (Serial2.available()){
    char c = (char)Serial2.read();
    if (c == '\n' || c == '\r'){
      if (ligneUart.length()){
        float v;
        if (extraireTension(ligneUart, &v)) derniereTension_V = v;
        ligneUart = "";
      }
    } else {
      ligneUart += c;
      if (ligneUart.length() > 60) ligneUart = "";
    }
  }
}
