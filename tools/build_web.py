"""Packs the WebAssembly core and the browser shell into one HTML file.
usage: build_web.py build/tini.wasm dist/index.html [--fragment]
--fragment leaves out the <html>/<head> wrapper, for hosts that add their own."""
import base64
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
web = os.path.join(here, "..", "web")


def main():
    wasm_path, out_path = sys.argv[1], sys.argv[2]
    html = open(os.path.join(web, "shell.html"), encoding="utf-8").read()
    js = open(os.path.join(web, "shell.js"), encoding="utf-8").read()
    b64 = base64.b64encode(open(wasm_path, "rb").read()).decode("ascii")
    html = html.replace("__WASM_BASE64__", b64).replace("/*__SHELL_JS__*/", js)
    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    # GitHub Pages and plain browsers need a full document; the shell is a fragment.
    if "--fragment" not in sys.argv and not html.lstrip().lower().startswith("<!doctype"):
        html = ('<!doctype html>\n<html lang="en">\n<head>\n<meta charset="utf-8">\n'
                '<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">\n'
                "</head>\n<body>\n" + html + "\n</body>\n</html>\n")
    open(out_path, "w", encoding="utf-8").write(html)
    print("wrote", out_path, len(html) // 1024, "KB")


if __name__ == "__main__":
    main()
