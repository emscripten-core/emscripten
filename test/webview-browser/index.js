#!/usr/bin/env node

const gi = require('node-gtk');
const Gtk = gi.require('Gtk', '3.0');
const WebKit2 = gi.require('WebKit2');

const [binary, script, ...args] = process.argv;

gi.startLoop();
Gtk.init();

const win = new Gtk.Window();
const webView = new WebKit2.WebView()

win.on('destroy', () => Gtk.mainQuit());
win.on('delete-event', () => false);

win.setDefaultSize(800, 600);
win.add(webView);

args[0] && webView.loadUri(args[0]);

win.showAll();
Gtk.main();
