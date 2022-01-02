import { createApp } from 'vue';
import {
  create,
  NAlert,
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
  NSlider,
  NGrid,
  NGi,
  NInputGroup,
} from 'naive-ui';
import App from './App.vue';

const naive = create({
  components: [
    NAlert,
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
    NSlider,
    NGrid,
    NGi,
    NInputGroup,
  ],
});

const app = createApp(App);
app.use(naive);

app.mount('#app');
