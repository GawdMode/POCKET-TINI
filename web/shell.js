// Browser shell for the TINI core: clock in, taps in, pixels out.
(async function () {
  const W = 368, H = 448, SAVE_KEY = "pocket-tini-save-v1";
  const $ = (id) => document.getElementById(id);
  const canvas = $("screen"), ctx = canvas.getContext("2d");

  const bytes = Uint8Array.from(atob(WASM_B64), (c) => c.charCodeAt(0));
  const { instance } = await WebAssembly.instantiate(bytes, {});
  const T = instance.exports;

  const cString = (ptr) => {
    const mem = new Uint8Array(T.memory.buffer);
    let end = ptr;
    while (mem[end]) end++;
    return new TextDecoder().decode(mem.subarray(ptr, end));
  };

  function randomSeed() {
    const a = new Uint32Array(1);
    crypto.getRandomValues(a);
    return a[0] || 1;
  }

  function save() {
    const ptr = T.tini_save_area();
    const n = T.tini_save(ptr, 64);
    const data = new Uint8Array(T.memory.buffer, ptr, n);
    try { localStorage.setItem(SAVE_KEY, btoa(String.fromCharCode(...data))); } catch (e) { /* storage blocked */ }
  }

  function load() {
    let text = null;
    try { text = localStorage.getItem(SAVE_KEY); } catch (e) { /* storage blocked */ }
    if (!text) return false;
    const data = Uint8Array.from(atob(text), (c) => c.charCodeAt(0));
    const ptr = T.tini_save_area();
    new Uint8Array(T.memory.buffer, ptr, data.length).set(data);
    return T.tini_load(ptr, data.length) === 1;
  }

  if (!load()) { T.tini_init(randomSeed()); save(); }

  // Test speed: game time runs faster and the Tini's clock moves ahead with it.
  let speed = 1, clockOffset = 0;
  const speeds = [1, 10, 60, 600];
  function setSpeed(s) {
    speed = s;
    for (const v of speeds) $("speed-" + v).setAttribute("aria-pressed", String(v === s));
  }
  for (const v of speeds) $("speed-" + v).addEventListener("click", () => setSpeed(v));
  $("clock-reset").addEventListener("click", () => { clockOffset = 0; setSpeed(1); });

  const newBtn = $("new-tini");
  let armed = null;
  newBtn.addEventListener("click", () => {
    if (!armed) {
      newBtn.textContent = "Tap again to replace " + cString(T.tini_name());
      armed = setTimeout(() => { armed = null; newBtn.textContent = "New random Tini"; }, 3000);
      return;
    }
    clearTimeout(armed); armed = null;
    newBtn.textContent = "New random Tini";
    T.tini_init(randomSeed());
    save();
  });

  canvas.addEventListener("pointerdown", (e) => {
    const r = canvas.getBoundingClientRect();
    const x = Math.round((e.clientX - r.left) / r.width * W);
    const y = Math.round((e.clientY - r.top) / r.height * H);
    T.tini_tap(x, y);
    save();
  });

  const image = ctx.createImageData(W, H);
  let last = performance.now(), lastSave = last;

  function frame(now) {
    const real = Math.min(now - last, 250);
    last = now;
    clockOffset += real * (speed - 1);
    const t = new Date(Date.now() + clockOffset);
    const minute = t.getHours() * 60 + t.getMinutes();
    T.tini_set_clock(minute);
    T.tini_update(Math.round(real * speed));
    T.tini_render();
    image.data.set(new Uint8Array(T.memory.buffer, T.tini_framebuffer(), W * H * 4));
    ctx.putImageData(image, 0, 0);

    $("name").textContent = cString(T.tini_name());
    $("status").textContent = cString(T.tini_status());
    $("clock").textContent = t.toLocaleTimeString([], { hour: "numeric", minute: "2-digit" }) + (clockOffset ? " (test time)" : "");
    if (now - lastSave > 5000) { save(); lastSave = now; }
    requestAnimationFrame(frame);
  }
  document.addEventListener("visibilitychange", () => { if (document.hidden) save(); });
  requestAnimationFrame(frame);
})().catch((err) => {
  document.getElementById("status").textContent = "Could not start: " + err.message;
});
