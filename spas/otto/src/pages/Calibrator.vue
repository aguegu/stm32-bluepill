<template>
  <v-container fluid>
    <v-card>
      <v-card-text>
        <v-row v-for="d, i in servos" :key="i">
          <span>#{{i}}</span>
          <pre>{{ d }}</pre>
        </v-row>
        <!-- <pre>{{ servos }}</pre> -->
      </v-card-text>
    </v-card>
  </v-container>
</template>

<script>
export default {
  name: 'PageCalibrator',
  data: () => ({
    servos: [{
      a0: 500,
      a90: 1500,
      a180: 2500,
    }, {
      a0: 500,
      a90: 1500,
      a180: 2500,
    }],
  }),
};
</script>
