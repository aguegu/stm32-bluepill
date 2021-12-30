module.exports = {
  env: {
    browser: true,
    es2021: true,
  },
  extends: [
    'plugin:vue/vue3-recommended',
    'airbnb-base',
  ],
  parserOptions: {
    ecmaVersion: 12,
    sourceType: 'module',
  },
  plugins: [
    'vue',
  ],
  rules: {
    'vue/max-attributes-per-line': ['error', {
      singleline: 8,
    }],
    'vue/singleline-html-element-content-newline': 'off',
    'max-len': 'off',
  },
};
