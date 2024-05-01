// @ts-check

import eslint from '@eslint/js';
import tseslint from 'typescript-eslint';
import globals from 'globals';
import react from 'eslint-plugin-react'
import reactHooks from 'eslint-plugin-react-hooks'

export default tseslint.config(
  eslint.configs.recommended,
  ...tseslint.configs.strict,
  ...tseslint.configs.stylistic,
  ...tseslint.configs.recommendedTypeChecked,
  {
    languageOptions: {
      parserOptions: {
        project: ['./frontend/tsconfig.json'],
        tsconfigRootDir: import.meta.dirname,
      },
      globals: {
        ...globals.browser,
      }
    },
    rules: {
      '@typescript-eslint/consistent-type-definitions': 'off',
      '@typescript-eslint/no-misused-promises': [
        'error',
        {
          'checksVoidReturn': {
            'attributes': false
          }
        }
      ]
    }
  },
  {
    files: ['**/*.{js,jsx,mjs,cjs,ts,tsx}'],
    plugins: {
      react,
      'react-hooks': reactHooks,
    },
    languageOptions: {
      ...react.configs.recommended.languageOptions,
    },
    rules: {
      ...react.configs.all.rules,
      'react/function-component-definition': ['error', { 'namedComponents': 'arrow-function' }],
      'react/jsx-filename-extension': ['error', { 'extensions': ['.tsx'] }],
      'react/jsx-indent': 'off',
      'react/jsx-indent-props': 'off',
      'react/jsx-sort-props': ['warn', { 'noSortAlphabetically': true }],
      'react/forbid-component-props': 'off',
      'react/jsx-max-props-per-line': ['warn', { 'maximum': 1, 'when': 'multiline' }],
      'react/jsx-max-depth': ['warn', { 'max': 4 }],
      'react/jsx-one-expression-per-line': ['warn', { 'allow': 'single-child' }],
      'react/jsx-no-literals': 'off',
      'react/jsx-newline': 'off',
      ...reactHooks.configs.recommended.rules,
    },
    settings: {
      react: {
        version: '18',
      }
    }
  }
);
