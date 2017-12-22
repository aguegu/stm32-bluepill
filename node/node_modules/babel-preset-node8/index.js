module.exports = {
  plugins: [
    require('babel-plugin-transform-es2015-modules-commonjs'),
    require('babel-plugin-transform-class-properties'),
    require('babel-plugin-transform-object-rest-spread'),
    require('babel-plugin-transform-export-extensions'),
    require('babel-plugin-transform-async-generator-functions'),
    require('babel-plugin-syntax-trailing-function-commas'),
  ],
  env: {
    production: {
      plugins: [
        require('babel-plugin-transform-es2015-block-scoping'),
        [require('babel-plugin-transform-es2015-for-of'), { loose: true }],
        require('babel-plugin-transform-inline-consecutive-adds'),
        require('babel-plugin-minify-dead-code-elimination'),
      ],
    },
  },
}
