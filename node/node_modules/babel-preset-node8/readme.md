# babel-preset-node8

## Babel 6+ presets for Node 7.10-8.x

Node v7.10 comes with native support of async/await. This preset for Babel 6 adds some remaining features.
Also plugin has plugins to build production-ready "fast" code.


## Included plugins

- [transform-es2015-modules-commonjs](http://babeljs.io/docs/plugins/transform-es2015-modules-commonjs)
- [transform-class-properties](http://babeljs.io/docs/plugins/transform-class-properties)
- [transform-object-rest-spread](http://babeljs.io/docs/plugins/transform-object-rest-spread)
- [transform-export-extensions](http://babeljs.io/docs/plugins/transform-export-extensions)
- [transform-async-generator-functions](http://babeljs.io/docs/plugins/transform-async-generator-functions)
- [syntax-trailing-function-commas](http://babeljs.io/docs/plugins/syntax-trailing-function-commas)

### Plugins in `env.production`

- [transform-es2015-block-scoping](http://babeljs.io/docs/plugins/transform-es2015-block-scoping)
- [transform-es2015-for-of](http://babeljs.io/docs/plugins/transform-es2015-for-of) with `{ "loose": true }`
- [transform-inline-consecutive-adds](http://babeljs.io/docs/plugins/transform-inline-consecutive-adds)
- [minify-dead-code-elimination](http://babeljs.io/docs/plugins/minify-dead-code-elimination)

## Installation

```bash
npm install --save-dev babel-preset-node8
```

## Usage

### Via `.babelrc` (recommended)

Create a `.babelrc` file in your project root, and include `node8` in your presets list.

```json
{
  "presets": ["node8"]
}
```
Now whenever you can use latest ECMAScript features with `babel-node` or `babel-cli`.

### Via CLI

```shell
babel source.js --presets node8
# or
babel ./src -d ./dist --presets node8
```

### Via Node API

```js
require('babel-core').transform('Your source code', {
  presets: ['node8'],
})
```

Also you can use `babel-register` to transpile in runtime:

```js
require('babel-register')
require('./path/to/es6-7/sources')
```

### Webpack, Rollup, Browserify, Gulp, etc.

Follow vendor instructions and include `node8` preset in your babel "preset" list (if needed).


## React support

Babel already has a preset for React, and you now need to install it separately.

```shell
npm install --save-dev babel-preset-react
```

And then add it to your "presets" list in `.babelrc` file:

```json
{
  "presets": [
    "node8",
    "react"
  ]
}
```

## Credits

Created by [Sergey Sova](https://lestad.top) [(LestaD)](https://github.com/lestad).
