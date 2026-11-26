#include <cstdlib>
#include <filesystem>
#include <iostream>
#include "app.h"

using namespace std;

// Usage: advisor [--data DIR] [--web DIR] [--port N]
//   --data  folder with the JSON data files      (default: current folder)
//   --web   built frontend to serve (web/dist)    (default: web/dist; skipped if missing)
//   --port  port on 127.0.0.1                     (default: 8080)
int main(int argc, char* argv[]) {
    cout << unitbuf;   // print log lines immediately (the server runs until killed)
    string dataDir = ".";
    string webDir  = "web/dist";
    int    port    = 8080;
    for (int i = 1; i + 1 < argc; i += 2) {
        string flag = argv[i], value = argv[i + 1];
        if      (flag == "--data") dataDir = value;
        else if (flag == "--web")  webDir  = value;
        else if (flag == "--port") port    = atoi(value.c_str());
        else { cerr << "unknown option " << flag << "\n"; return 1; }
    }

    AppData app = loadAppData(dataDir);
    cout << "Loaded " << app.catalog.size() << " courses from " << dataDir << "\n";
    for (const string& e : app.dataErrors) cout << "  data: " << e << "\n";

    httplib::Server server;
    registerRoutes(server, app);

    if (filesystem::is_directory(webDir)) {
        server.set_mount_point("/", webDir);
        cout << "Serving frontend from " << webDir << "\n";
    } else {
        cout << "No frontend at " << webDir << " (run `npm run build` in web/, or use `npm run dev`)\n";
    }

    cout << "Listening on http://127.0.0.1:" << port << "\n";
    if (!server.listen("127.0.0.1", port)) {
        cerr << "could not listen on port " << port << " (already in use?)\n";
        return 1;
    }
    return 0;
}
