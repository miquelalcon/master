#include "normal_kd_tree.h"

ostream & operator << (std::ostream & out, const Point & p) {
    out << "(";
    out << p[0];
    for (int j = 1; j < p.size(); ++j)
        out << "," << p[j];
    out << ")";
    return out;
}

ostream & operator << (std::ostream & out, const PointVector & v) {
    for (int i = 0; i < v.size(); ++i) {
        out << (v[i]);
    }
    return out;
}

KDTree::KDTree (const uint &dim) {
    this->dim = dim;
}

KDTree::KDTree (const uint &dim, const PointVector &v) {
    this->dim = dim;
    if (this->check_dimension(v)) {
        this->build_standard(v);
    }
    else
        throw string("Dimensions does not match.");
}

bool KDTree::check_dimension(const PointVector &v) {
    for (const Point element : v) {
        if (element.size() != this->dim)
            return false;
    }
    return true;
}

ostream & operator << (std::ostream & out, const KDTree & kdt) {
    out << "( " << kdt.axis << " | " << kdt.location[0];
    for (int i = 1; i < kdt.location.size(); ++i) {
        out << "," << kdt.location[i];
    }
    out << " )";
    out << kdt.left << kdt.right;
    return out;
}

ostream & operator << (std::ostream & out, const KDTree* kdt) {
    if (kdt != NULL) {
        out << endl << "( " << kdt->axis ;
        out << " | " << kdt->location[0];
        for (int i = 1; i < kdt->location.size(); ++i) {
            out << "," << kdt->location[i];
        }
        out << " )";
        out << kdt->left << kdt->right;
    }
    else
        out << "*";
    return out;
}

Point KDTree::qselect (const uint &k, const uint &axis, PointVector v) {
    Point pivot = v[rand() % v.size()];
    PointVector left, right;
    uint elements_like_pivot = 0;
    for (Point element: v) {
        if (element[axis] < pivot[axis])
            left.push_back(element);
        else if (element[axis] > pivot[axis])
            right.push_back(element);
        else
            ++elements_like_pivot;
    }
    if (k < left.size())
        return qselect(k, axis, left);
    else if (k >= left.size() + elements_like_pivot)
        return qselect(k-left.size()-elements_like_pivot, axis, right);
    else
        return pivot;
}

//Point KDTree::median(const uint &axis, PointVector v) {
//    uint median_pos;
//    if (v.size() % 2 == 0) 
//        median_pos = v.size()/2;
//    else
//        median_pos = (v.size()-1)/2;
//    auto comp = [&](Point x, Point y) {return x[axis] < y[axis];};
//    sort(v.begin(), v.end(), comp);
//    print_point_vector(v);
//    return v[median_pos];
//}

Point KDTree::median (const uint &axis, PointVector v) {
    uint median_pos;
    if (v.size() % 2 == 0) 
        median_pos = v.size()/2;
    else
        median_pos = (v.size()-1)/2;
    return KDTree::qselect(median_pos, axis, v);
}

void KDTree::build_standard (const PointVector &v) {
    if (!v.empty()) {
        this->dim = uint(dim);
        this->axis = rand() % dim;
        this->location = KDTree::median(this->axis,v);
        PointVector left, right;
        for (auto element: v) {
            if (element != this->location) {
                if (element[this->axis] <= this->location[this->axis])
                    left.push_back(element);
                else
                    right.push_back(element);
            }
        }
        if (!left.empty()) {
            this->left = new KDTree(this->dim);
            this->left->build_standard(left);
        }
        else
            this->left = NULL;

        if (!right.empty()) {
            this->right = new KDTree(this->dim);
            this->right->build_standard(right);
        }
        else
            this->right = NULL;
    }
    else {
        this->axis = 0;
        this->location = Point (this->dim,0);
        this->left = NULL;
        this->right = NULL;
    }
}

float KDTree::distance(const Point &p, const Point &q) {
    float sum = 0;
    for (int i = 0; i < p.size(); ++i) {
        sum += pow(q[i] - p[i], 2);
    }
    return sum;
}

void KDTree::recursive_search(float &best_distance, Point &best_p, const Point &p) {
    float d = distance(p, this->location);
    if (d < best_distance) {
        best_distance = d;
        best_p = this->location;
    }
    if (this->left != NULL)
        if (p[this->axis] <= this->location[this->axis])
            this->left->recursive_search(best_distance, best_p, p);
    else if (this->right != NULL)
        if (p[this->axis] > this->location[this->axis])
            this->right->recursive_search(best_distance, best_p, p);
}

Point KDTree::search(const Point &p) {
    if (this->dim == p.size()) {
        Point best_p = this->location;
        float d = distance(p, best_p);
        if (p[this->axis] <= this->location[this->axis])
            this->left->recursive_search(d, best_p, p);
        else
            this->right->recursive_search(d, best_p, p);
        return best_p;
    }
    else {
        throw string("Searched point has different dimension.");
        return Point (0);
    }
}

int main() {
    srand (time(NULL));
    try {
        KDTree t (2, {{5,2},{9,5},{2,6},{4,3},{3,4},{6,1}});
        cout << t << endl;
        Point p = t.search({9,4});
        cout << p <<  endl;
    }
    catch (string s) {
        cout << s << endl;
    }
}