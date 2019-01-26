# Contributing to Astring

Suggestions and changes are welcome and can be made through issue tickets and pull requests.

## Scripts

* `npm run build`: Produces a JavaScript 5-compliant modules at `dist/astring.js`.
* `npm run build:minified`: Produces a JavaScript 5-compliant modules at `dist/astring.min.js` along with a source map at `dist/astring.min.js.map`.
* `npm start`: Generates `dist/astring.js` and `dist/astring.js.map` at each change detected on `src/astring.js`.
* `npm test`: Runs tests.
* `npm run coverage`: Runs tests with coverage.
* `npm run test:scripts`: Runs tests over a large array of script files.
* `npm run benchmark`: Runs benchmarks against other code generators. Requires to run `npm install escodegen@1.8 uglify-js@2 babel-generator@6 buble@0.15` first.

## Roadmap

Planned features and releases are outlined on the [milestones page](https://github.com/davidbonnet/astring/milestones).
