import React from 'react';
import { createRoot } from 'react-dom/client';
import App from './App';

window.addEventListener('beforeunload', (event) => {
  event.preventDefault();
});

const rootContainer = document.getElementById('app-root');

if (rootContainer !== null) {
  const root = createRoot(rootContainer);

  root.render(
    <React.StrictMode>
      <App />
    </React.StrictMode>
  );
}
