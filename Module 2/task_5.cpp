/*
Алгоритм сжатия данных Хаффмана
*/

#include "Huffman.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <queue>
#include <map>
#include <unordered_map>
#include <algorithm>

class BitWriter {
private:
    IOutputStream& output;
    byte buffer;          
    int bitsInBuffer;     
    
public:
    BitWriter(IOutputStream& out) : output(out), buffer(0), bitsInBuffer(0) {}
    
    void writeBit(bool bit) {
        buffer = (buffer << 1) | (bit ? 1 : 0);
        bitsInBuffer++;
        
        if (bitsInBuffer == 8) {
            output.Write(buffer);
            buffer = 0;
            bitsInBuffer = 0;
        }
    }
    
    void writeBits(unsigned value, int numBits) {
        for (int i = numBits - 1; i >= 0; i--) {
            writeBit((value >> i) & 1);
        }
    }
    
    void writeByte(byte value) {
        writeBits(value, 8);
    }
    
    void flush() {
        if (bitsInBuffer > 0) {
            buffer <<= (8 - bitsInBuffer);
            output.Write(buffer);
        }
    }
    
    ~BitWriter() {
        flush();
    }
};

class BitReader {
private:
    IInputStream& input;
    byte buffer;          
    int bitsInBuffer;     
    bool endOfStream;     
    
public:
    BitReader(IInputStream& in) : input(in), buffer(0), bitsInBuffer(0), endOfStream(false) {}
    
    bool readBit() {
        if (bitsInBuffer == 0) {
            if (!input.Read(buffer)) {
                endOfStream = true;
                return false; 
            }
            bitsInBuffer = 8;
        }
        
        bool bit = (buffer >> (bitsInBuffer - 1)) & 1;
        bitsInBuffer--;
        
        return bit;
    }
    
    unsigned readBits(int numBits) {
        unsigned value = 0;
        
        for (int i = 0; i < numBits; i++) {
            if (endOfStream) {
                return value;
            }
            value = (value << 1) | (readBit() ? 1 : 0);
        }
        
        return value;
    }
    
    byte readByte() {
        return readBits(8);
    }
    
    bool isEndOfStream() const {
        return endOfStream;
    }
};

class BufferedInputStream : public IInputStream {
private:
    IInputStream& source;
    std::vector<byte> buffer;
    size_t position;
    bool buffered;
    
public:
    BufferedInputStream(IInputStream& src) : source(src), position(0), buffered(false) {}
    
    void bufferInput() {
        if (buffered) return; 

	byte value;
        while (source.Read(value)) {
            buffer.push_back(value);
        }
        buffered = true;
        position = 0;
    }
    
    bool Read(byte& value) override {
        if (!buffered) {
            bufferInput();
        }
        
        if (position >= buffer.size()) {
            return false;
        }
        
        value = buffer[position++];
        return true;
    }
    
    void rewind() {
        position = 0;
    }
    
    size_t size() const {
        if (!buffered) {
            const_cast<BufferedInputStream*>(this)->bufferInput(); 
        }
        return buffer.size();
    }
    
    const std::vector<byte>& getData() const {
        if (!buffered) {
            const_cast<BufferedInputStream*>(this)->bufferInput(); 
        }
        return buffer;
    }
    
    void setData(const std::vector<byte>& newData) {
        buffer = newData;
        position = 0;
        buffered = true;
    }
};

class HuffmanTree {
private:
    struct Node {
        byte value;
        unsigned frequency;
        Node* left;
        Node* right;
        
        Node(byte val, unsigned freq) 
            : value(val), frequency(freq), left(nullptr), right(nullptr) {}
        
        Node(unsigned freq, Node* l, Node* r) 
            : value(0), frequency(freq), left(l), right(r) {}
        
        bool isLeaf() const { 
            return left == nullptr && right == nullptr; 
        }
        
        ~Node() {
            delete left;
            delete right;
        }
    };
    
    struct CompareNodes {
        bool operator()(Node* a, Node* b) {
            return a->frequency > b->frequency;
        }
    };
    
    Node* root;
    std::unordered_map<byte, std::string> codes;
    
    void generateCodes(Node* node, std::string code) {
        if (node == nullptr) return;
        
        if (node->isLeaf()) {
            if (code.empty()) {
                code = "0";
            }
            codes[node->value] = code;
        } else {
            generateCodes(node->left, code + "0");
            generateCodes(node->right, code + "1");
        }
    }
    
    void writeTree(Node* node, BitWriter& writer) const {
        if (node == nullptr) return;
        
        writer.writeBit(node->isLeaf());
        
        if (node->isLeaf()) {
            writer.writeByte(node->value);
        } else {
            writeTree(node->left, writer);
            writeTree(node->right, writer);
        }
    }
    
    Node* readTree(BitReader& reader) const {
        if (reader.isEndOfStream()) {
            return nullptr;
        }
        
        bool isLeaf = reader.readBit();
        
        if (isLeaf) {
            byte value = reader.readByte();
            return new Node(value, 0);
        } else {
            Node* left = readTree(reader);
            Node* right = readTree(reader);
            return new Node(0, left, right);
        }
    }
    
public:
    HuffmanTree() : root(nullptr) {}
    
    ~HuffmanTree() {
        clear();
    }
    
    void clear() {
        delete root;
        root = nullptr;
        codes.clear();
    }
    
    void buildFromFrequencies(const std::map<byte, unsigned>& frequencies) {
        clear(); 
        
        std::priority_queue<Node*, std::vector<Node*>, CompareNodes> pq;
        
        for (const auto& pair : frequencies) {
            pq.push(new Node(pair.first, pair.second));
        }
        
        if (pq.empty()) {
            return;
        }
        
        if (pq.size() == 1) {
            Node* leaf = pq.top();
            pq.pop();
            root = new Node(leaf->frequency + 1, leaf, new Node(0, 1));
            generateCodes(root, "");
            return;
        }
        
        while (pq.size() > 1) {
            Node* left = pq.top();
            pq.pop();
            
            Node* right = pq.top();
            pq.pop();
            
            unsigned sumFreq = left->frequency + right->frequency;
            pq.push(new Node(sumFreq, left, right));
        }
        
        root = pq.top();
        
        generateCodes(root, "");
    }
    
    bool isBuilt() const {
        return root != nullptr;
    }
    
    std::string getCode(byte symbol) const {
        auto it = codes.find(symbol);
        if (it != codes.end()) {
            return it->second;
        }
        return "";
    }
    
    void serialize(BitWriter& writer) const {
        writeTree(root, writer);
    }
    
    void deserialize(BitReader& reader) {
        clear();
        root = readTree(reader);
        generateCodes(root, "");
    }
    
    void encode(BufferedInputStream& input, BitWriter& writer) const {
        if (!isBuilt()) {
            return;
        }
        
        input.rewind();
        byte value;
        
        while (input.Read(value)) {
            const std::string& code = getCode(value);
            for (char bit : code) {
                writer.writeBit(bit == '1');
            }
        }
    }
    
    void decode(BitReader& reader, IOutputStream& output, size_t originalSize) const {
        if (!isBuilt()) {
            return;
        }
        
        Node* current = root;
        size_t bytesDecoded = 0;
        
        while (bytesDecoded < originalSize && !reader.isEndOfStream()) {
            bool bit = reader.readBit();
            
            if (bit) {
                current = current->right;
            } else {
                current = current->left;
            }
            
            if (current && current->isLeaf()) {
                output.Write(current->value);
                bytesDecoded++;
                current = root;
            }
        }
    }
    
    static std::map<byte, unsigned> countFrequencies(IInputStream& input) {
        std::map<byte, unsigned> frequencies;
        byte value;
        
        while (input.Read(value)) {
            frequencies[value]++;
        }
        
        return frequencies;
    }
};

void Encode(IInputStream& original, IOutputStream& compressed)
{
    BufferedInputStream bufferedInput(original);
    bufferedInput.bufferInput();
    
    const std::vector<byte>& originalData = bufferedInput.getData();
    if (originalData.empty()) {
        return;
    }
    
    bufferedInput.rewind();
    std::map<byte, unsigned> frequencies = HuffmanTree::countFrequencies(bufferedInput);
    
    HuffmanTree huffmanTree;
    huffmanTree.buildFromFrequencies(frequencies);
    
    if (!huffmanTree.isBuilt()) {
        return;
    }
    
    BitWriter writer(compressed);
    
    size_t originalSize = originalData.size();
    writer.writeBits((originalSize >> 24) & 0xFF, 8);
    writer.writeBits((originalSize >> 16) & 0xFF, 8);
    writer.writeBits((originalSize >> 8) & 0xFF, 8);
    writer.writeBits(originalSize & 0xFF, 8);
    
    huffmanTree.serialize(writer);
    
    bufferedInput.rewind();
    huffmanTree.encode(bufferedInput, writer);
    
    writer.flush();
}

void Decode(IInputStream& compressed, IOutputStream& original)
{
    BitReader reader(compressed);
    
    unsigned originalSize = 0;
    originalSize = (reader.readByte() << 24) | 
                   (reader.readByte() << 16) | 
                   (reader.readByte() << 8) | 
                   reader.readByte();
    
    HuffmanTree huffmanTree;
    huffmanTree.deserialize(reader);
    
    if (!huffmanTree.isBuilt()) {
        return;
    }
    
    huffmanTree.decode(reader, original, originalSize);
}