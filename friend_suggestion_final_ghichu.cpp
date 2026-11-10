// =====================================================================
//  SOCIAL NETWORK FRIEND SUGGESTION  (BẢN CÓ GHI CHÚ ĐỂ HỌC)
//  - vector<User> : danh sách người dùng cơ bản (Array/List)
//  - BST          : tìm người dùng nhanh theo ID
//                   (khóa = ID, giá trị = vị trí của user trong vector)
//  Nếu comment tiếng Việt bị lỗi font trong Dev-C++: Edit > Encoding > UTF-8
// =====================================================================
#include <algorithm>      // find, sort
#include <cstdlib>        // exit
#include <fstream>        // ifstream, ofstream: đọc/ghi file
#include <iostream>       // cin, cout
#include <sstream>        // stringstream: tách chuỗi theo dấu | và ,
#include <string>         // string
#include <unordered_map>  // bảng băm: đếm số bạn chung
#include <unordered_set>  // tập băm: kiểm tra "đã là bạn chưa" nhanh
#include <vector>         // mảng động
using namespace std;

// ---------------------------------------------------------------------
// 1. STRUCT USER: một người dùng trong mạng xã hội
// ---------------------------------------------------------------------
struct User {
    int id;                    // mã người dùng, duy nhất (không trùng)
    string name;               // tên người dùng
    vector<int> friends;       // danh sách ID của bạn bè (chỉ lưu ID, không lưu cả User)

    // Kiểm tra user này đã là bạn với user có ID = f chưa. Độ phức tạp O(d), d = số bạn
    bool hasFriend(int f) const {
        // find duyệt từ đầu đến cuối vector, tìm thấy thì khác end()
        return find(friends.begin(), friends.end(), f) != friends.end();
    }
};

// ---------------------------------------------------------------------
// 2. STRUCT NODE: một nút của cây BST
//    Nút KHÔNG chứa cả User, chỉ chứa ID và vị trí của User trong vector
//    (giống "mục lục" của quyển sách: tra ID -> biết trang nào)
// ---------------------------------------------------------------------
struct Node {
    int id;                    // khóa tìm kiếm của BST
    int idx;                   // vị trí (chỉ số) của user này trong vector users
    Node *left, *right;        // con trái (ID nhỏ hơn), con phải (ID lớn hơn)
    // Hàm khởi tạo: gán id, idx; chưa có con nên để nullptr (con trỏ rỗng)
    Node(int id, int idx) : id(id), idx(idx), left(nullptr), right(nullptr) {}
};

// ---------------------------------------------------------------------
// 3. CLASS SOCIALNETWORK: quản lý toàn bộ mạng xã hội
// ---------------------------------------------------------------------
class SocialNetwork {
    vector<User> users;        // DANH SÁCH người dùng (Array/List), lưu theo thứ tự thêm vào
    Node* root = nullptr;      // gốc của cây BST, ban đầu cây rỗng

    // ---- Chèn một ID vào BST. O(h), h = chiều cao cây ----
    // Dấu &  trong "Node*& n": truyền THAM CHIẾU tới con trỏ, để khi gán
    // n = new Node(...) thì con trỏ thật của nút cha (hoặc root) cũng được cập nhật.
    void insert(Node*& n, int id, int idx) {
        if (!n) n = new Node(id, idx);                  // chỗ trống -> tạo nút mới tại đây
        else insert(id < n->id ? n->left : n->right,    // nhỏ hơn -> sang trái, lớn hơn -> sang phải
                    id, idx);                           // gọi đệ quy tiếp
    }

    // ---- Tìm nút có ID cho trước. O(h). Trả về nullptr nếu không có ----
    Node* search(int id) const {
        Node* n = root;                                 // bắt đầu từ gốc
        while (n && n->id != id)                        // còn nút và chưa đúng ID thì đi tiếp
            n = id < n->id ? n->left : n->right;        // nhỏ hơn đi trái, lớn hơn đi phải
        return n;                                       // tìm thấy: nút đó; không thấy: nullptr
    }

    // ---- Giải phóng toàn bộ nút của cây (tránh rò rỉ bộ nhớ) ----
    // Xóa con trái, con phải trước rồi mới xóa nút hiện tại (hậu thứ tự)
    void destroy(Node* n) {
        if (n) { destroy(n->left); destroy(n->right); delete n; }
    }

public:
    // Destructor: tự chạy khi đối tượng bị hủy, dọn cây BST
    ~SocialNetwork() { destroy(root); }

    // ---- THÊM NGƯỜI DÙNG ----
    bool addUser(int id, const string& name) {
        if (id <= 0 || name.empty() || search(id)) return false; // ID không hợp lệ, tên rỗng hoặc ID đã tồn tại
        users.push_back(User{id, name, {}});            // 1) cất User vào cuối vector (chưa có bạn: {})
        insert(root, id, (int)users.size() - 1);        // 2) ghi vào BST: ID này nằm ở vị trí cuối vector
        return true;
    }

    // ---- TÌM NGƯỜI DÙNG THEO ID ----
    // Tìm trong BST (nhanh) -> lấy idx -> truy cập users[idx]
    User* searchUser(int id) {
        Node* n = search(id);
        return n ? &users[n->idx] : nullptr;            // có -> địa chỉ User; không -> nullptr
    }

    // ---- KẾT BẠN (quan hệ 2 chiều) ----
    bool addFriendship(int a, int b) {
        User *x = searchUser(a), *y = searchUser(b);    // tìm cả hai người
        if (!x || !y || a == b || x->hasFriend(b))      // thiếu 1 người / tự kết bạn với mình / đã là bạn
            return false;
        x->friends.push_back(b);                        // a thêm b vào danh sách bạn
        y->friends.push_back(a);                        // b thêm a vào danh sách bạn (hai chiều)
        return true;
    }

    // ---- GỢI Ý BẠN BÈ (hàm quan trọng nhất) ----
    // Trả về danh sách (ID ứng viên, số bạn chung), nhiều bạn chung đứng trước
    vector<pair<int, int>> suggestFriends(int id) {
        vector<pair<int, int>> res;                     // kết quả trả về
        User* u = searchUser(id);                       // người cần gợi ý
        if (!u) return res;                             // không tồn tại -> trả danh sách rỗng

        // Tập bạn trực tiếp: kiểm tra "đã là bạn chưa" trong O(1) trung bình
        unordered_set<int> direct(u->friends.begin(), u->friends.end());
        // Bảng đếm: cnt[ID ứng viên] = số bạn chung với u
        unordered_map<int, int> cnt;

        for (int f : u->friends)                        // duyệt từng BẠN của u (tầng 1)
            if (User* fu = searchUser(f))               //   lấy thông tin người bạn f
                for (int c : fu->friends)               //   duyệt từng BẠN CỦA BẠN (tầng 2)
                    if (c != id && !direct.count(c))    //   bỏ qua chính u và những người đã là bạn của u
                        cnt[c]++;                       //   c thêm 1 bạn chung (chính là f)

        res.assign(cnt.begin(), cnt.end());             // chuyển bảng đếm thành danh sách để sắp xếp
        sort(res.begin(), res.end(),                    // sắp xếp bằng hàm so sánh tự viết (lambda) bên dưới
             [](const pair<int, int>& x, const pair<int, int>& y) {
                 // Số bạn chung khác nhau: nhiều hơn đứng trước.
                 // Bằng nhau: ID nhỏ hơn đứng trước (thứ tự ổn định, giống mục 2.1 trong report)
                 return x.second != y.second ? x.second > y.second : x.first < y.first;
             });
        return res;
    }

    // ---- LƯU FILE ----
    // Mỗi dòng: USER|id|tên|id bạn 1,id bạn 2,...
    bool save(const string& fn) const {
        ofstream o(fn);                                 // mở file để ghi
        if (!o) return false;                           // không mở được -> báo lỗi
        for (const User& u : users) {                   // ghi theo thứ tự thêm vào => load lại BST giống hệt
            o << "USER|" << u.id << "|" << u.name << "|";
            for (size_t i = 0; i < u.friends.size(); i++)
                o << (i ? "," : "") << u.friends[i];    // dấu phẩy ngăn cách, phần tử đầu không có dấu phẩy
            o << "\n";
        }
        return true;
    }

    // ---- ĐỌC FILE ---- (trả về số user đọc được, -1 nếu không mở được file)
    int load(const string& fn) {
        ifstream in(fn);                                // mở file để đọc
        if (!in) return -1;
        destroy(root);                                  // xóa dữ liệu cũ trong chương trình
        root = nullptr;
        users.clear();

        vector<pair<int, vector<int>>> rows;            // tạm lưu (ID, danh sách ID bạn) của từng dòng
        string line, tag, sid, name, fl, t;
        while (getline(in, line)) {                     // đọc từng dòng của file
            stringstream ss(line);
            // Tách theo dấu '|': "USER" | id | tên | danh sách bạn. Dòng sai định dạng -> bỏ qua
            if (!getline(ss, tag, '|') || tag != "USER" ||
                !getline(ss, sid, '|') || !getline(ss, name, '|')) continue;
            getline(ss, fl);                            // phần còn lại: "20,30"
            int id;
            try { id = stoi(sid); } catch (...) { continue; }   // chuỗi -> số; lỗi thì bỏ dòng
            vector<int> fr;
            stringstream fs(fl);
            while (getline(fs, t, ','))                 // tách danh sách bạn theo dấu ','
                try { fr.push_back(stoi(t)); } catch (...) {}
            if (addUser(id, name)) rows.push_back({id, fr});   // LƯỢT 1: tạo user
        }
        // LƯỢT 2: nối bạn bè. Phải làm sau khi tạo hết user, vì dòng đầu có thể
        // ghi bạn là người ở dòng sau (lúc đó người kia chưa được tạo).
        for (auto& r : rows)
            for (int f : r.second) addFriendship(r.first, f);
        return (int)rows.size();
    }
};

// ---------------------------------------------------------------------
// 4. HÀM NHẬP SỐ AN TOÀN: nhập chữ cũng không làm chương trình treo
// ---------------------------------------------------------------------
int readInt(const char* msg) {
    int v;
    cout << msg;                                        // in câu hỏi
    while (!(cin >> v)) {                               // cin >> v thất bại nghĩa là người dùng nhập không phải số
        if (cin.eof()) exit(0);                         // hết dữ liệu nhập thì thoát
        cin.clear();                                    // xóa trạng thái lỗi của cin
        cin.ignore(1000, '\n');                         // bỏ phần nhập sai
        cout << "Hay nhap so nguyen. " << msg;          // hỏi lại
    }
    cin.ignore(1000, '\n');                             // bỏ dấu Enter còn sót (để getline đọc tên không bị trôi)
    return v;
}

// ---------------------------------------------------------------------
// 5. HÀM MAIN: vòng lặp menu
//    Dòng "Choice:" chỉ nhận số 1-8. ID, tên... chỉ nhập khi chương trình hỏi riêng.
// ---------------------------------------------------------------------
int main() {
    SocialNetwork net;                                  // tạo mạng xã hội rỗng
    int c;                                              // lựa chọn của người dùng
    do {
        cout << "\n1. Add user\n2. Search user\n3. Add friendship\n4. Display friends\n"
                "5. Suggest friends\n6. Save\n7. Load\n8. Exit\n";
        c = readInt("Choice: ");

        if (c == 1) {                                   // --- Thêm người dùng ---
            int id = readInt("ID: ");
            string name;
            cout << "Name: ";
            getline(cin, name);                         // getline để tên có khoảng trắng vẫn đọc được
            cout << (net.addUser(id, name) ? "Added.\n" : "Failed (trung ID hoac du lieu sai).\n");

        } else if (c == 2) {                            // --- Tìm người dùng theo ID ---
            User* u = net.searchUser(readInt("ID: "));
            if (u) cout << "Found: " << u->name << ", " << u->friends.size() << " friends\n";
            else cout << "Not found.\n";

        } else if (c == 3) {                            // --- Kết bạn ---
            int a = readInt("First ID: ");
            int b = readInt("Second ID: ");
            cout << (net.addFriendship(a, b) ? "Added.\n" : "Failed (ID khong ton tai hoac da la ban).\n");

        } else if (c == 4) {                            // --- Hiển thị bạn bè ---
            User* u = net.searchUser(readInt("ID: "));
            if (!u) { cout << "Not found.\n"; continue; }
            cout << "Friends of " << u->name << ":\n";
            for (int f : u->friends)                    // mỗi ID bạn -> tra tên bằng BST
                cout << "  " << f << " - " << net.searchUser(f)->name << "\n";

        } else if (c == 5) {                            // --- Gợi ý bạn bè ---
            int id = readInt("ID: ");
            User* u = net.searchUser(id);
            if (!u) { cout << "Not found.\n"; continue; }
            auto s = net.suggestFriends(id);            // auto: tự suy ra kiểu vector<pair<int,int>>
            if (s.empty()) cout << "No suggestions.\n";
            for (auto& p : s)                           // p.first = ID ứng viên, p.second = số bạn chung
                cout << "  " << p.first << " - " << net.searchUser(p.first)->name
                     << " (mutual: " << p.second << ")\n";

        } else if (c == 6) {                            // --- Lưu file ---
            string fn;
            cout << "File: ";
            getline(cin, fn);
            cout << (net.save(fn) ? "Saved.\n" : "Cannot open file.\n");

        } else if (c == 7) {                            // --- Đọc file ---
            string fn;
            cout << "File: ";
            getline(cin, fn);
            int n = net.load(fn);
            if (n < 0) cout << "Cannot open file.\n";
            else cout << "Loaded " << n << " users.\n";

        } else if (c != 8) {                            // không phải 1..8
            cout << "Invalid choice.\n";
        }
    } while (c != 8);                                   // chọn 8 thì thoát vòng lặp
    return 0;
}
