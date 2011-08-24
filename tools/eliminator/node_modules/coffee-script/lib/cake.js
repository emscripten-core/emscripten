(function() {
  var CoffeeScript, fs, helpers, missingTask, oparse, options, optparse, path, printTasks, switches, tasks;
  fs = require('fs');
  path = require('path');
  helpers = require('./helpers');
  optparse = require('./optparse');
  CoffeeScript = require('./coffee-script');
  tasks = {};
  options = {};
  switches = [];
  oparse = null;
  helpers.extend(global, {
    task: function(name, description, action) {
      var _ref;
      if (!action) {
        _ref = [description, action], action = _ref[0], description = _ref[1];
      }
      return tasks[name] = {
        name: name,
        description: description,
        action: action
      };
    },
    option: function(letter, flag, description) {
      return switches.push([letter, flag, description]);
    },
    invoke: function(name) {
      if (!tasks[name]) {
        missingTask(name);
      }
      return tasks[name].action(options);
    }
  });
  exports.run = function() {
    return path.exists('Cakefile', function(exists) {
      var arg, args, _i, _len, _ref, _results;
      if (!exists) {
        throw new Error("Cakefile not found in " + (process.cwd()));
      }
      args = process.argv.slice(2);
      CoffeeScript.run(fs.readFileSync('Cakefile').toString(), {
        filename: 'Cakefile'
      });
      oparse = new optparse.OptionParser(switches);
      if (!args.length) {
        return printTasks();
      }
      options = oparse.parse(args);
      _ref = options.arguments;
      _results = [];
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        arg = _ref[_i];
        _results.push(invoke(arg));
      }
      return _results;
    });
  };
  printTasks = function() {
    var desc, name, spaces, task;
    console.log('');
    for (name in tasks) {
      task = tasks[name];
      spaces = 20 - name.length;
      spaces = spaces > 0 ? Array(spaces + 1).join(' ') : '';
      desc = task.description ? "# " + task.description : '';
      console.log("cake " + name + spaces + " " + desc);
    }
    if (switches.length) {
      return console.log(oparse.help());
    }
  };
  missingTask = function(task) {
    console.log("No such task: \"" + task + "\"");
    return process.exit(1);
  };
}).call(this);
