import { createApp } from 'vue';
import {
  create,
  NButton,
  NLayout,
  NLayoutHeader,
  NLayoutContent,
  NPageHeader,
  NH1,
  NCard,
  NConfigProvider,
  NInput,
  NInputNumber,
  NSpace,
  NTag,
} from 'naive-ui';
import App from './App.vue';

const naive = create({
  components: [NButton, NLayout, NLayoutHeader, NLayoutContent, NPageHeader, NH1, NCard, NConfigProvider, NInput, NInputNumber, NSpace, NTag],
});

const app = createApp(App);
app.use(naive);

app.mount('#app');
