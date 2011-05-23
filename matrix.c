#include "matrix.h"

#include <vector>
#include "../util.h"

Matrix::Matrix(std::string m) {
  // Idenfity matrix
  val[0] = 1; val[1] = 0; val[2] = 0;
  val[3] = 1; val[4] = 0; val[5] = 0;

  if(!m.empty()) {
    using namespace std;
    string::size_type loc = m.rfind("matrix");

    if(loc != string::npos) {
      // Set new matrix
      m.erase(0, 6);
      vector<string> v(tokenize_str(m));
      int num_tokens = v.size();
      assert(num_tokens == 6);
      for (unsigned i  = 0; i < num_tokens; i++)
        val[i] = atof(v[i].c_str());
    } else {
      // Translate identity matrix
      m.erase(0, 9);
      vector<string> v(tokenize_str(m));
      int num_tokens = v.size();
      assert(num_tokens == 2);
      val[4] = atof(v[0].c_str());
      val[5] = atof(v[1].c_str());
    }
  }
}
