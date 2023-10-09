#pragma once
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <iostream>

/**
 * @brief left-leaning Red Black Tree only holding pointers
 *
 * @tparam key_type
 * @tparam val_type
 */
template <typename key_type, typename val_type>
class red_black_BST
{
    enum class COLOR
    {
        BLACK = 0,
        RED,
    };
    struct node
    {
        key_type *key;
        val_type *val;
        node *left=nullptr, *right =nullptr;
        size_t size; // sub tree size
        COLOR color;
        node(key_type *key, val_type *val, COLOR c, size_t size) :
            key(key), val(val), color(c), size(size)
        {
        }
    };

    node *root = nullptr;

    bool is_red(node *n)
    {
        if (n == nullptr) return false;

        return n->color == COLOR::RED;
    }
    size_t size(node *x)
    {
        if (x == nullptr) return 0;
        return x->size;
    }

    val_type *get(node *x, key_type *key)
    {
        while (x != nullptr)
        {
            auto cmp = (*key - *x->key); // rewrite the key compare method
            if (cmp < 0)
                x = x->left;
            else if (cmp > 0)
                x = x->right;
            else
                return x->val;
        }
        return nullptr;
    }

    node *put(node *h, key_type *key, val_type *val)
    {
        if (h == nullptr) return new node{key, val, COLOR::RED, 1};

        auto cmp = (*key - *h->key); // as same as previous

        if (cmp < 0)
            h->left = put(h->left, key, val);
        else if (cmp > 0)
            h->right = put(h->right, key, val);
        else
            h->val = val;

        // fix-up any right-leaning links
        if (is_red(h->right) && !is_red(h->left)) h = rotate_left(h);
        if (is_red(h->left) && is_red(h->left->left)) h = rotate_right(h);
        if (is_red(h->left) && is_red(h->right)) flip_colors(h);

        h->size = this->size(h->left) + this->size(h->right) + 1;

        return h;
    }

    node *remove_min(node *h)
    {
        if (h->left == nullptr)
        {
            delete h; // delete h node and return nullptr,this nullptr value will be assigned to h
            return nullptr;
        }

        if (!is_red(h->left) && !is_red(h->left->left))
            h = move_red_left(h);

        h->left = remove_min(h->left);
        return balance(h);
    }

    node *remove_max(node *h)
    {
        if (is_red(h->left))
            h = rotate_right(h);

        if (h->right == nullptr)
        {
            delete h;
            return nullptr;
        }

        if (!is_red(h->right) && !is_red(h->right->left))
            h = move_red_right(h);

        h->right = remove_max(h->right);

        return balance(h);
    }

    node *remove(node *h, key_type *key)
    {
        if (*key - *h->key < 0)
        {
            if (!is_red(h->left) && !is_red(h->left->left))
                h = move_red_left(h);
            h->left = remove(h->left, key);
        }
        else
        {
            if (is_red(h->left))
                h = rotate_right(h);
            if (*key - *h->key == 0 && (h->right == nullptr))
                return nullptr;

            if (!is_red(h->right) && !is_red(h->right->left))
                h = move_red_right(h);

            if (*key - *h->key == 0)
            {
                node *x = min(h->right);

                h->key = x->key;

                h->val = x->val;

                h->right = remove_min(h->right);
            }
            else
                h->right = remove(h->right, key);
        }
        return balance(h);
    }

    node *rotate_right(node *h)
    {
        assert(h != nullptr && is_red(h->left));

        node *x = h->left;
        h->left = x->right;
        x->right = h;
        x->color = h->color;
        h->color = COLOR::RED;
        x->size = h->size;
        h->size = this->size(h->left) + this->size(h->right) + 1;
        return x;
    }

    node *rotate_left(node *h)
    {
        assert(h != nullptr && is_red(h->right));

        node *x = h->right;
        h->right = x->left;
        x->left = h;
        x->color = h->color;
        h->color = COLOR::RED;
        x->size = h->size;
        h->size = this->size(h->left) + this->size(h->right) + 1;
        return x;
    }

    void help_flip_color(node *h)
    {
        if (h->color == COLOR::BLACK)
            h->color = COLOR::RED;
        else
            h->color = COLOR::BLACK;
    }

    void flip_colors(node *h)
    {
        help_flip_color(h);
        help_flip_color(h->left);
        help_flip_color(h->right);
    }

    node *move_red_left(node *h)
    {
        flip_colors(h);
        if (is_red(h->right->left))
        {
            h->right = rotate_right(h->right);
            h = rotate_left(h);
            flip_colors(h);
        }
        return h;
    }

    node *move_red_right(node *h)
    {
        flip_colors(h);
        if (is_red(h->left->left))
        {
            h = rotate_left(h);
            flip_colors(h);
        }
        return h;
    }

    node *balance(node *h)
    {
        if (is_red(h->right) && !is_red(h->left)) h = rotate_left(h);
        if (is_red(h->left) && is_red(h->left->left)) h = rotate_right(h);
        if (is_red(h->left) && is_red(h->right)) flip_colors(h);

        h->size = this->size(h->left) + this->size(h->right) + 1;
        return h;
    }
    size_t height(node *x)
    {
        if (x == nullptr) return -1;

        return 1 + std::max(height(x->left), height(x->right));
    }

    node *min(node *x)
    {
        if (x->left == nullptr)
            return x;
        else
            return min(x->left);
    }
    node *max(node *x)
    {
        if (x->right == nullptr)
            return x;
        else
            return max(x->right);
    }

    bool is_BST(node *x, key_type *min, key_type *max)
    {
        if (x == nullptr) return true;
        if (min != nullptr && *x->key - *min <= 0) return false;
        if (max != nullptr && *x->key - *max >= 0) return false;

        return is_BST(x->left, min, x->key) && is_BST(x->right, x->key, max);
    }

    bool is_balanced(node *x, size_t black)
    {
        if (x == nullptr) return black == 0;

        if (!is_red(x)) black--;
        return is_balanced(x->left, black) && is_balanced(x->right, black);
    }

  public:
    size_t size()
    {
        return size(root);
    }
    bool is_empty()
    {
        return root == nullptr;
    }
    val_type *get(key_type *key)
    {
        assert(key != nullptr);

        return get(root, key);
    }

    bool contains(key_type *key)
    {
        return get(key) != nullptr;
    }

    void put(key_type *key, val_type *val)
    {
        assert(key != nullptr);

        if (val == nullptr)
        {
            this->remove(key);
            return;
        }

        root = put(root, key, val);

        root->color = COLOR::BLACK;
    }

    void remove_min()
    {
        assert(!is_empty());

        if (!is_red(root->left) && !is_red(root->right))
            root->color = COLOR::RED;

        root = remove_min(root);

        if (!is_empty()) root->color = COLOR::BLACK;
    }

    void remove_max()
    {
        assert(!is_empty());

        if (!is_red(root->left) && !is_red(root->right))
            root->color = COLOR::RED;

        root = remove_max(root);

        if (!is_empty()) root->color = COLOR::BLACK;
    }
    void remove(key_type *key)
    {
        assert(key != nullptr);
        if (!contains(key)) return;

        if (!is_red(root->left) && !is_red(root->right))
            root->color = COLOR::RED;
        root = remove(root, key);

        if (!is_empty()) root->color = COLOR::BLACK;
    }

    size_t height()
    {
        return height(root);
    }
    key_type *min()
    {
        assert(!is_empty());
        return min(root)->key;
    }
    key_type *max()
    {
        assert(!is_empty());

        return max(root)->key;
    }

    bool is_BST()
    {
        return is_BST(root, nullptr, nullptr);
    }

    bool is_balanced()
    {
        size_t black = 0;
        node *x = root;
        while (x != nullptr)
        {
            if (!is_red(x)) black++;
            x = x->left;
        }
        return is_balanced(root, black);
    }
};
//TODO:keys iterator
//TODO:clear
