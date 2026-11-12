<template>
  <v-app>
    <v-app-bar app>
      <v-app-bar-title>Otto Robot Servo Calibrator</v-app-bar-title>
    </v-app-bar>
    <v-main>
      <Calibrator/>
    </v-main>
  </v-app>
</template>

<script>
import Calibrator from './pages/Calibrator.vue';

export default {
  name: 'App',

  components: {
    Calibrator,
  },

  data: () => ({
    //
  }),
};
</script>
