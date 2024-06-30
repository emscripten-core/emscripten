#!/usr/bin/env node

const gi = require('node-gtk');
const Gtk = gi.require('Gtk', '3.0');
const WebKit2 = gi.require('WebKit2');

const [binary, script, ...args] = process.argv;
const [url] = args;

gi.startLoop();
Gtk.init();

const win = new Gtk.Window();
const webView = new WebKit2.WebView();

webView.getSettings().enableDeveloperExtras = true;

win.on('destroy', () => Gtk.mainQuit());
win.on('delete-event', () => false);

win.setDefaultSize(800, 600);
win.add(webView);

url && webView.loadUri(url);

win.showAll();
Gtk.main();
