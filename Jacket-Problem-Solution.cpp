/**
 * Composite Jacket Thermal Resistance - C++ Solution
 * Time Complexity: O(Q * log(MAX_D0)) for Type 4 queries
 * Space Complexity: O(N) where N = 6 layers
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

const double EPS = 1e-9;
const double T_S = 37.0;      // Body temperature (°C)
const double T_EXT = -13.0;   // Ambient temperature (°C)
const double Q_MAX = 20.0;    // Maximum heat flux (W/m²)
const double MOISTURE_RATE = 0.01;  // Moisture per hour

class Layer {
public:
    double d;      // thickness (meters)
    double k;      // base conductivity (W/m·K)
    double mu;     // moisture coefficient
    double c;      // compression coefficient
    double W;      // accumulated moisture
    double C;      // accumulated compression
    
    Layer(double d = 0, double k = 0, double mu = 0, double c = 0) 
        : d(d), k(k), mu(mu), c(c), W(0.0), C(0.0) {}
    
    // Calculate effective conductivity
    double k_eff(double beta) const {
        return k * (1.0 + mu * W) * exp(beta * C);
    }
    
    // Calculate thermal resistance of this layer
    double thermal_resistance(double beta) const {
        if (d < EPS) return 0.0;
        return d / k_eff(beta);
    }
};

class ThermalJacketSystem {
private:
    vector<Layer> layers;
    double beta;
    
    // Calculate total thermal resistance excluding a specific layer
    double calculate_R_th_without_layer(int exclude_layer) {
        double R_th = 0.0;
        for (int i = 0; i < 7; i++) {
            if (i != exclude_layer) {
                R_th += layers[i].thermal_resistance(beta);
            }
        }
        return R_th;
    }
    
    // Calculate total thermal resistance
    double calculate_R_th() {
        double R_th = 0.0;
        for (int i = 0; i < 7; i++) {
            R_th += layers[i].thermal_resistance(beta);
        }
        return R_th;
    }
    
    // Calculate heat flux for a given d0 (layer 0 thickness)
    double heat_flux(double d0) {
        // Calculate R_th with layer 0 having thickness d0
        double R_th = calculate_R_th_without_layer(1);  // layer index 1 is layer 0
        
        // Add layer 0's contribution
        if (d0 > EPS) {
            double k0_eff = layers[1].k_eff(beta);
            R_th += d0 / k0_eff;
        }
        
        // q = ΔT / R_th
        double delta_T = T_S - T_EXT;  // 50°C
        if (R_th < 1e-12) return 1e18;
        return delta_T / R_th;
    }
    
public:
    ThermalJacketSystem() : layers(7), beta(0.0) {}
    
    // Read initial layer properties and beta
    void read_input() {
        // Read 7 layers (index -1 to 5)
        for (int i = 0; i < 7; i++) {
            double d, k, mu, c;
            cin >> d >> k >> mu >> c;
            layers[i] = Layer(d, k, mu, c);
        }
        
        // Read beta
        cin >> beta;
    }
    
    // Type 1: Simulate t hours of snowfall
    void query_type1_environmental_exposure(double t) {
        // Add moisture to layer 5 (index 6)
        layers[6].W += t * MOISTURE_RATE;
        
        // Propagate moisture inward (layer 5 to layer -1)
        for (int i = 6; i > 0; i--) {
            Layer& layer_i = layers[i];
            Layer& layer_prev = layers[i - 1];
            // W_(i-1) += W_i × μ_i
            layer_prev.W += layer_i.W * layer_i.mu;
        }
    }
    
    // Type 2: Apply compression stress x to layer i
    void query_type2_mechanical_stress(int i, double x) {
        // Convert layer index (-1 to 5) to array index (0 to 6)
        int layer_idx = i + 1;
        
        // Add compression to layer i
        layers[layer_idx].C += x;
        
        // Propagate compression outward (layer i to layer 5)
        for (int idx = layer_idx; idx < 6; idx++) {
            Layer& layer_curr = layers[idx];
            Layer& layer_next = layers[idx + 1];
            // C_(i+1) += C_i × c_i
            layer_next.C += layer_curr.C * layer_curr.c;
        }
    }
    
    // Type 3: Replace all properties of layer i
    void query_type3_replace_layer(int i, double d, double k, double mu, double c) {
        // Convert layer index (-1 to 5) to array index (0 to 6)
        int layer_idx = i + 1;
        
        // Replace properties
        layers[layer_idx].d = d;
        layers[layer_idx].k = k;
        layers[layer_idx].mu = mu;
        layers[layer_idx].c = c;
        
        // Reset accumulated values
        layers[layer_idx].W = 0.0;
        layers[layer_idx].C = 0.0;
    }
    
    // Type 4: Calculate minimum d0 such that q ≤ 20
    double query_type4_minimum_foam_thickness() {
        // Binary search on d0
        double left = 0.0;
        double right = 1e9;
        
        // Check if even d0 = 0 satisfies the constraint
        if (heat_flux(0.0) <= Q_MAX + EPS) {
            return 0.0;
        }
        
        // Check if even large d0 cannot satisfy
        if (heat_flux(right) > Q_MAX + EPS) {
            return 1e18;
        }
        
        // Binary search
        int iterations = 0;
        const int MAX_ITERATIONS = 100;
        
        while (right - left > 1e-8 && iterations < MAX_ITERATIONS) {
            double mid = (left + right) / 2.0;
            double q = heat_flux(mid);
            
            if (q <= Q_MAX + 1e-10) {
                right = mid;
            } else {
                left = mid;
            }
            
            iterations++;
        }
        
        return right;
    }
    
    // Type 5: Check if any finite d0 can satisfy the constraint
    string query_type5_feasibility_check() {
        // Check with d0 = 0
        double q_min = heat_flux(0.0);
        if (q_min <= Q_MAX + EPS) {
            return "POSSIBLE";
        }
        
        // Check with very large d0
        double q_with_large_d0 = heat_flux(1e9);
        if (q_with_large_d0 <= Q_MAX + EPS) {
            return "POSSIBLE";
        }
        
        // Check the trend
        double q1 = heat_flux(1e6);
        double q2 = heat_flux(1e7);
        
        if (q2 < q1 && q2 <= Q_MAX + EPS) {
            return "POSSIBLE";
        }
        
        // If heat flux is still too high with very large d0
        if (q_with_large_d0 > Q_MAX - EPS) {
            return "IMPOSSIBLE";
        }
        
        return "POSSIBLE";
    }
    
    // Process all queries
    void process_queries() {
        int Q;
        cin >> Q;
        
        for (int q = 0; q < Q; q++) {
            int query_type;
            cin >> query_type;
            
            if (query_type == 1) {
                // Environmental exposure
                double t;
                cin >> t;
                query_type1_environmental_exposure(t);
            }
            else if (query_type == 2) {
                // Mechanical stress
                int i;
                double x;
                cin >> i >> x;
                query_type2_mechanical_stress(i, x);
            }
            else if (query_type == 3) {
                // Replace layer
                int i;
                double d, k, mu, c;
                cin >> i >> d >> k >> mu >> c;
                query_type3_replace_layer(i, d, k, mu, c);
            }
            else if (query_type == 4) {
                // Minimum foam thickness
                double d0 = query_type4_minimum_foam_thickness();
                cout << fixed << setprecision(10) << d0 << "\n";
            }
            else if (query_type == 5) {
                // Feasibility check
                string result = query_type5_feasibility_check();
                cout << result << "\n";
            }
        }
    }
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    
    ThermalJacketSystem system;
    system.read_input();
    system.process_queries();
    
    return 0;
}

/*
COMPILATION:
g++ -std=c++17 -O2 -Wall solution.cpp -o solution

RUN:
./solution < input.txt

SAMPLE INPUT:
0.02 0.04 0.1 0.2
0.05 0.03 0.15 0.25
0.0 0.035 0.12 0.22
0.0 0.038 0.11 0.21
0.015 0.045 0.13 0.23
0.018 0.042 0.14 0.24
0.001 0.25 0.05 0.1
0.5
5
4
1 10
4
2 3 5.0
4

KEY OPTIMIZATIONS:
1. Fast I/O with ios_base::sync_with_stdio(false)
2. Efficient memory usage with vector
3. Inline calculations where possible
4. Binary search with optimal epsilon
5. Direct mathematical formulas

COMPLEXITY:
- Time: O(Q * log(MAX_D0) * N)
- Space: O(N) where N = 7
*/
