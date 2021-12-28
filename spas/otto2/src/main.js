import { createApp } from 'vue';
import {
  // create naive ui
  create,
  // component
  NButton,
} from 'naive-ui';
import App from './App.vue';

const naive = create({
  components: [NButton],
});

const app = createApp(App);
app.use(naive);

app.mount('#app');
