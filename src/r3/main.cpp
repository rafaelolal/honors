#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <numbers>
#include <random>

using namespace std;

// constexpr is const and known at compile time??
// 50 meters
constexpr int SCALE = 50;
constexpr int N = 10;
constexpr double PI = 3.141592653589793;

struct Point {
  int y;
  int x;
};

Point viewpoint{6, 6};

// Parameters aren't of type point to keep functions generalizable
double distance(double first_y, double first_x, double second_y,
                double second_x) {
  // AI recommended this function
  return hypot(second_y - first_y, second_x - first_x);
}

double zinuth(double viewpoint_y, double viewpoint_x, double viewpoint_height,
              double target_y, double target_x, double target_height) {
  cout << "viewpoint_y: " << viewpoint_y << " viewpoint_x: " << viewpoint_x
       << " target_y: " << target_y << " target_x: " << target_x << endl;
  double d = distance(viewpoint_y, viewpoint_x, target_y, target_x);
  cout << "zinuth d: " << d << endl;
  if (d == 0) {
    return 0;
  }

  // TOA
  // tan(z) = opposite / adjacent
  // z = atan(opposite / adjacent)
  double z = atan((target_height - viewpoint_height) / d);
  cout << "zinuth z: " << z << endl;
  return z;
}

// This method looks more confusing than it has to be because I made it
// interpolate either direction. Not sure if there is a more elegant way to do
// it. Value is the y or x depending on interpolate_y coordinate is the other
// value in the pair that is exactly known
double interpolate(int interpolate_y, double value, int coordinate,
                   const double (&dem)[N][N]) {
  assert(interpolate_y == 0 || interpolate_y == 1);
  assert(coordinate >= 0 && coordinate < N);

  assert(value >= 0);
  int v_below = static_cast<int>(value);
  int v_above = v_below + 1;
  assert(v_below >= 0 && v_below < N);
  assert(v_above >= 0 && v_above < N);

  int t = value - v_below;
  assert(t >= 0 && t <= 1);
  int y_below, y_above, x_below, x_above;
  if (interpolate_y == 1) {
    y_below = v_below;
    y_above = v_above;
    x_below = coordinate;
    x_above = coordinate;
  } else {
    y_below = coordinate;
    y_above = coordinate;
    x_below = v_below;
    x_above = v_above;
  }

  return dem[y_below][x_below] * (1.0 - t) + dem[y_above][x_above] * t;
}

int is_visible(const Point& viewpoint, const Point& target,
               const double (&dem)[N][N]) {
  assert(viewpoint.y >= 0 && viewpoint.y < N);
  assert(viewpoint.x >= 0 && viewpoint.x < N);
  assert(target.y >= 0 && target.y < N);
  assert(target.x >= 0 && target.x < N);

  // zinuth from viewpoint to target
  double theta = zinuth(viewpoint.y, viewpoint.x, dem[viewpoint.y][viewpoint.x],
                        target.y, target.x, dem[target.y][target.x]);
  // 90 degrees
  assert(theta >= -PI / 2 && theta <= PI / 2);

  // horizontal angle from viewpoint to target
  int dy = target.y - viewpoint.y;
  int dx = target.x - viewpoint.x;
  double alpha = atan(static_cast<double>(dy) / static_cast<double>(dx));
  // 90 degrees
  // TODO: double check if this is correct
  // assert(alpha >= -PI / 2 && alpha <= PI / 2);

  cout << "viewpoint: (" << viewpoint.y << ", " << viewpoint.x << ") target: ("
       << target.y << ", " << target.x << ") alpha: " << alpha
       << " theta: " << theta << endl;

  // Vertical intersections
  int dir_x = dx > 0 ? 1 : -1;
  // Start one away in the direction of target, and move towards it
  cout << "y_calcs" << endl;
  // TODO: need to immediately get rid of this if!!!!!
  if (viewpoint.x == target.x) {
    cout << "Vertical line, skipping vertical intersections" << endl;
  } else {
    for (int x = viewpoint.x; x != target.x; x += dir_x) {
      // y is ray vertical intersection point y-value
      // Negative angles means I am going backwards, just take abs instead

      double y_calc =
          static_cast<double>(x - viewpoint.x) * tan(alpha) + viewpoint.y;
      double height = interpolate(1, y_calc, x, dem);
      assert(height >= 0);

      double z = zinuth(viewpoint.y, viewpoint.x, dem[viewpoint.y][viewpoint.x],
                        y_calc, x, height);
      cout << "z: " << z << endl;
      // 90 degrees
      assert(z >= -PI / 2 && z <= PI / 2);

      if (z > theta) {
        return 0;
      }
    }
  }

  int dir_y = target.y - viewpoint.y > 0 ? 1 : -1;
  // Horizontal intersections
  // Start one away in the direction of target, and move towards it
  cout << "x_calcs" << endl;
  // TODO: need to immediately get rid of this if!!!!!
  if (viewpoint.y == target.y) {
    cout << "Horizontal line, skipping horizontal intersections" << endl;
  } else {
    for (int y = viewpoint.y; y != target.y; y += dir_y) {
      // x is ray horizontal intersection point x-value
      double x_calc =
          static_cast<double>(y - viewpoint.y) / tan(alpha) + viewpoint.x;
      cout << "calculated x: " << x_calc << " for y: " << y
           << " alpha: " << alpha << endl;
      double height = interpolate(0, x_calc, y, dem);
      assert(height >= 0);

      double z = zinuth(viewpoint.y, viewpoint.x, dem[viewpoint.y][viewpoint.x],
                        y, x_calc, height);
      cout << "z: " << z << endl;
      // 90 degrees
      assert(z >= -PI / 2 && z <= PI / 2);

      if (z > theta) {
        return 0;
      }
    }
  }

  return 1;
}

int main(int argc, char* argv[]) {
  double dem[N][N] = {};

  mt19937 generator(0);
  uniform_real_distribution<double> elevation(1.0, 4.0);

  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      // Scale the elevation values to simplify calculations later
      dem[y][x] = elevation(generator) / SCALE;
    }
  }

  dem[6][6] = 10;

  int viewshed[N][N] = {};

  cout << "Starting viewshed analysis with viewpoint at (" << viewpoint.y
       << ", " << viewpoint.x << ")\n";
  for (int y = 0; y < N; y++) {
    for (int x = 0; x < N; x++) {
      Point target{y, x};
      viewshed[y][x] = is_visible(viewpoint, target, dem);
    }
  }
  cout << "Viewshed analysis complete\n";

  for (int y = 0; y < N; y++) {
    for (int x = 0; x < N; x++) {
      cout << dem[y][x] << ' ';
    }

    cout << endl;
  }

  cout << endl;
  for (int y = 0; y < N; y++) {
    for (int x = 0; x < N; x++) {
      if (y == viewpoint.y && x == viewpoint.x) {
        cout << 'X';
      } else {
        cout << viewshed[y][x];
      }

      cout << ' ';
    }

    cout << endl;
  }

  return 0;
}