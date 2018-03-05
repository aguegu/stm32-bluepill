#include "easing.h"

const double PI = acos(-1);

double easeLinear(double t) {
  return t;
}

double easeSineIn(double t) {
  return sin((t - 1) * PI / 2) + 1;
}

double easeSineOut(double t) {
  return sin(t * PI / 2);
}

double easeSineInOut(double t) {
  return 0.5 * (1 - cos(t * PI));
}

double easeQuadIn(double t) {
  return t * t;
}

double easeQuadOut(double t) {
  return t * (2 - t);
}

double easeQuadInOut(double t) {
  if (t < 0.5) {
    return 2 * t * t;
  }
  return (-2 * t * t) + (4 * t) - 1;
}

double easeCubicIn(double t) {
  return t * t * t;
}

double easeCubicOut(double t) {
  return (t - 1) * (t - 1) * (t - 1) + 1;
}

double easeCubicInOut(double t) {
  if (t < 0.5) {
    return 4 * t * t * t;
  }
  double p = 2 * t - 2;
  return 0.5 * p * p * p + 1;
}

double easeQuarticIn(double t) {
  return t * t * t * t;
}

double easeQuarticOut(double t) {
  return (t - 1) * (t - 1) * (t - 1) * (1 - t) + 1;
}

double easeQuarticInOut(double t) {
  if (t < 0.5) {
    return 8 * t * t * t * t;
  }
  double p = t - 1;
  return 1 - 8 * p * p * p * p;
}

double easeExponentialIn(double t) {
  if (t == 0.) {
    return 0.;
  }
  return exp(10 * (t - 1));
}

double easeExponentialOut(double t) {
  if (t == 1.) {
    return 1.;
  }
  return 1 - exp(-10 * t);
}

double easeExponentialInOut(double t) {
  if (t == 0. || t == 1.) {
    return t;
  }

  if (t < 0.5) {
    return 0.5 * exp(20 * t - 10);
  }
  return 1 - 0.5 * exp(10 - 20 * t);
}

double easeCircularIn(double t) {
  return 1 - sqrt(1 - t * t);
}

double easeCircularOut(double t) {
  return sqrt((2 - t) * t);
}

double easeCircularInOut(double t) {
  if (t < 0.5) {
    return 0.5 * (1 - sqrt(1 - 4 * t * t));
  }
  return 0.5 * (sqrt((3 - 2 * t) * (2 * t - 1)) + 1);
}

double easeBackIn(double t) {
  return t * t * t - t * sin(t * PI);
}

double easeBackOut(double t) {
  double p = 1.0 - t;
  return 1 - (p * p * p - p * sin(p * PI));
}

double easeBackInOut(double t) {
  double p;
  if (t < 0.5) {
    p = 2 * t;
    return 0.5 * (p * p * p - p * sin(p * PI));
  }

  p = 1 - (2 * t - 1);
  return 0.5 * (1 - (p * p * p - p * sin(p * PI))) + 0.5;
}

double easeElasticIn(double t) {
  return sin(13 * PI / 2 * t) * exp(10 * (t - 1));
}

double easeElasticOut(double t) {
  return sin(-13 * PI / 2 * (t + 1)) * exp(-10 * t) + 1;
}

double easeElasticInOut(double t) {
  if (t < 0.5) {
    return 0.5 * sin(13 * PI / 2 * (2 * t)) * exp(10 * ((2 * t) - 1));
  }
  return 0.5 * (sin(-13 * PI / 2 * ((2 * t - 1) + 1)) * exp(-10 * (2 * t - 1)) + 2);
}

double easeBounceOut(double t) {
  if (t < 4.0 / 11.0) {
    return 121.0 * t * t / 16.0;
  } else if (t < 8.0 / 11.0) {
    return (363.0 / 40.0 * t * t) - (99.0 / 10.0 * t) + 17.0 / 5.0;
  } else if (t < 9.0 / 10.0) {
    return (4356.0 / 361.0 * t * t) - (35442.0 / 1805.0 * t) + 16061.0 / 1805.0;
  }
  return (54.0 / 5.0 * t * t) - (513.0 / 25.0 * t) + 268.0 / 25.0;
}

double easeBounceIn(double t) {
  return 1 - easeBounceOut(1 - t);
}

double easeBounceInOut(double t) {
  if (t < 0.5) {
    return 0.5 * easeBounceIn (2 * t);
  }
  return 0.5 * easeBounceOut(2 * t - 1) + 0.5;
}

double oscillate(double t) {
  return t; // fake func
}

double (*EASING[28])(double) = {
  easeLinear,
  easeSineIn,
  easeSineOut,
  easeSineInOut,
  easeQuadIn,
  easeQuadOut,
  easeQuadInOut,
  easeCubicIn,
  easeCubicOut,
  easeCubicInOut,
  easeQuarticIn,
  easeQuarticOut,
  easeQuarticInOut,
  easeExponentialIn,
  easeExponentialOut,
  easeExponentialInOut,
  easeCircularIn,
  easeCircularOut,
  easeCircularInOut,
  easeBackIn,
  easeBackOut,
  easeBackInOut,
  easeElasticIn,
  easeElasticOut,
  easeElasticInOut,
  easeBounceIn,
  easeBounceOut,
  easeBounceInOut,
};
