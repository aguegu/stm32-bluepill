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
  NInputGroupLabel,
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
    NInputGroupLabel,
  ],
});

const app = createApp(App);
app.use(naive);

app.mount('#app');
