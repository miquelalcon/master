#include <ilcplex/ilocplex.h>
#include <math.h> 
#include <vector>
#include <fstream>
ILOSTLBEGIN

#define MAX_DEPTH 5

bool is_bit_up(const unsigned int &bit_pos, const int &number) {
  return ((unsigned int) pow(2, bit_pos) & number) != 0;
}

vector<int> read_input() {
  int n;
  cin >> n;
  vector<int> truth_table(pow(2,n));
  for (int i = 0; i < pow(2,n); ++i)
    cin >> truth_table[i];
  return truth_table;
}

IloNumVar var(IloNumVarArray &V, const int &h, const int &w)  {
  if (w >= pow(2,h))
    cerr << "ERR: Trying to access an invalid node." << endl;
  return V[(pow(2,h)-1)+w];
}

// Variable at the left
IloNumVar lvar(IloNumVarArray &V, const int &h, const int &w)  {
  return var(V, h+1, 2*w);
}

// Variable at the right
IloNumVar rvar(IloNumVarArray &V, const int &h, const int &w)  {
  return var(V, h+1, 2*w+1);
}

IloNumVar var(IloNumVarArray &V, const int &h, const int &w, const int &i, const int &extra)  {
  if (w >= pow(2,h))
    cerr << "ERR: Trying to access an invalid node." << endl;
  return V[extra*i + (pow(2,h)-1)+w];
}

// Variable at the left
IloNumVar lvar(IloNumVarArray &V, const int &h, const int &w, const int &i, const int &extra)  {
  return var(V, h+1, 2*w, i, extra);
}

// Variable at the right
IloNumVar rvar(IloNumVarArray &V, const int &h, const int &w, const int &i, const int &extra)  {
  return var(V, h+1, 2*w+1, i, extra);
}

void define_constraints(const int &h, const int &w, const int &depth, const int &t, 
                        const int &n, const int &cs, IloNumVarArray &Z, IloNumVarArray &I, 
                        IloNumVarArray &N, IloNumVarArray &B, IloModel &model, IloEnv &env) {
  if (h == depth) {
    model.add( var(N,h,w) == 0 );
  }
   else {
    define_constraints(h+1,   2*w, depth, t, n, cs, Z, I, N, B, model, env);
    define_constraints(h+1, 2*w+1, depth, t, n, cs, Z, I, N, B, model, env);
    // Force children of root to be 0 if root is not a NOR gate 
    model.add( lvar(Z,h,w) + rvar(Z,h,w) >= (1-var(N,h,w))*2 );
    // Force non-symmetry
    // TODO

    // Link -1 with the value according to the functionality of a NOR gate
    model.add( (1 - rvar(B,h,w,t,cs) - lvar(B,h,w,t,cs)) - 2*var(B,h,w,t,cs) <= 1-var(N,h,w) );
    model.add( (1 - rvar(B,h,w,t,cs) - lvar(B,h,w,t,cs)) - 2*var(B,h,w,t,cs) >= 2*var(N,h,w)-3 );    
  }

  // Link 0 with value 0
  model.add( 1 - var(Z,h,w) >= var(B,h,w,t,cs) );

  // Link each variable with its value according to truth_idx (idx of the row of the truth table)
  for (int i = 0; i < n; ++i) {
    model.add( 1 - var(I,h,w,i,cs) >= var(B,h,w,t,cs) - is_bit_up(n-i-1,t) );
    model.add( 1 - var(I,h,w,i,cs) >= is_bit_up(n-i-1,t) - var(B,h,w,t,cs) );
  }

  // Each node must be only of one type
  IloExpr unique(env);
  unique += var(Z,h,w) + var(N,h,w);
  for (int i = 0; i < n; ++i)
     unique += var(I,h,w,i,cs);
  model.add( unique == 1 );
}


int main () {

  const vector<int> truth_table = read_input();
  const int n = log2(truth_table.size());
  for (int depth = 0; depth <= MAX_DEPTH; ++depth) {
    //cout << "================ " <<  depth << " ================" << endl;
    IloEnv env;
    IloModel model(env);

    const int circuit_size = pow(2,depth+1)-1;

    IloNumVarArray Z(env, circuit_size, 0, 1, ILOINT);                    // Zero
    IloNumVarArray N(env, circuit_size, 0, 1, ILOINT);                    // NOR
    IloNumVarArray I(env, circuit_size*n, 0, 1, ILOINT);                  // Input
    IloNumVarArray B(env, circuit_size*truth_table.size(), 0, 1, ILOINT); // Circuit bool
    Z.setNames("Z");
    N.setNames("N");
    I.setNames("I");
    B.setNames("B");

    for (int truth_idx = 0; truth_idx < truth_table.size(); ++truth_idx) {
      define_constraints(0, 0, depth, truth_idx, n, circuit_size, Z, I, N, B, model, env);
      model.add( var(B,0,0,truth_idx,circuit_size) == truth_table[truth_idx] );
    }

    IloExpr size(env);
    for (int i = 0; i < circuit_size; ++i)
      size += N[i];
    model.add( IloMinimize(env, size) );

    cerr << model << endl;
    IloCplex cplex(model);
    cplex.setOut(env.getNullStream()); // remove CPLEX messages
    if (cplex.solve()) {
      cerr << cplex.getObjValue() << endl;
      cerr << "SOLVED " << depth << endl;
      env.end();
      break;
    }
    else {
      cerr << "INFEASIBLE " << depth << endl;
    }
    env.end();
  }

  
}
