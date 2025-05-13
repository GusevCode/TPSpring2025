/*                   
Текущая реализация представляет собой модификацию алгоритма 
bzip2 и использует следующую цепочку преобразований:
            Кодирование длин серий ->
            Преобразование Барроуза-Уилера ->
            Преобразование MTF ->
            Кодирование длин серий -> 
            Хаффман
 
Модификация алгоритма RLE не только для нулей улучшило на 3к баллов контест 
TODO: отправлять в BWT батчи исходного потока 
 ,----.                                     
'  .-./   ,--.,--. ,---.  ,---.,--.  ,--.   
|  | .---.|  ||  |(  .-' | .-. :\  `'  /    
'  '--'  |'  ''  '.-'  `)\   --. \    /.--. 
 `------'  `----' `----'  `----'  `--' '--' 
*/

#include "Huffman.h"
#include <vector>
#include <queue>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <string>

std::vector<byte> moveToFrontEncode(const std::vector<byte>& input) {
    std::vector<byte> alphabet(256);
    for (int i = 0; i < 256; ++i) {
        alphabet[i] = static_cast<byte>(i);
    }
    
    std::vector<byte> output;
    output.reserve(input.size());

    for (byte b : input) {
        auto it = std::find(alphabet.begin(), alphabet.end(), b);
        size_t position = std::distance(alphabet.begin(), it);
        
        output.push_back(static_cast<byte>(position));
        
        if (position > 0) {
            alphabet.erase(it);
            alphabet.insert(alphabet.begin(), b);
        }
    }
    
    return output;
}

std::vector<byte> moveToFrontDecode(const std::vector<byte>& input) {
    std::vector<byte> alphabet(256);
    for (int i = 0; i < 256; ++i) {
        alphabet[i] = static_cast<byte>(i);
    }
    
    std::vector<byte> output;
    output.reserve(input.size());
    
    for (byte position : input) {
        byte value = alphabet[position];

        output.push_back(value);
        
        if (position > 0) {
            alphabet.erase(alphabet.begin() + position);
            alphabet.insert(alphabet.begin(), value);
        }
    }
    
    return output;
}

struct HuffmanNode {
    byte value;          
    unsigned frequency;  
    HuffmanNode* left;  
    HuffmanNode* right;  
    
    HuffmanNode(byte val, unsigned freq) 
        : value(val), frequency(freq), left(nullptr), right(nullptr) {}
    
    HuffmanNode(unsigned freq, HuffmanNode* l, HuffmanNode* r) 
        : value(0), frequency(freq), left(l), right(r) {}
    
    bool isLeaf() const { 
        return left == nullptr && right == nullptr; 
    }
};

struct CompareNodes {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->frequency > b->frequency;
    }
};

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

std::vector<byte> runLengthEncode(const std::vector<byte>& input) {
    std::vector<byte> output;
    
    const byte RLE_MARKER = 0xFF;
    const size_t MIN_SEQUENCE = 4; 
    
    size_t i = 0;
    while (i < input.size()) {
        byte current = input[i];
        size_t count = 1;
        
        while (i + count < input.size() && input[i + count] == current && count < 255) {
            count++;
        }
        
        if (count >= MIN_SEQUENCE || (current == RLE_MARKER && count >= 2)) {
            output.push_back(RLE_MARKER);    
            output.push_back(current);    
            output.push_back(static_cast<byte>(count)); 
            i += count;
        } else if (current == RLE_MARKER) {
            output.push_back(RLE_MARKER);
            output.push_back(RLE_MARKER);
            output.push_back(1);
            i++;
        } else {
            output.push_back(current);
            i++;
        }
    }
    
    return output;
}

std::vector<byte> runLengthDecode(const std::vector<byte>& input) {
    std::vector<byte> output;
    
    const byte RLE_MARKER = 0xFF;
    
    size_t i = 0;
    while (i < input.size()) {
        if (input[i] == RLE_MARKER && i + 2 < input.size()) {
            byte symbol = input[i + 1];
            byte count = input[i + 2];
            
            for (byte j = 0; j < count; j++) {
                output.push_back(symbol);
            }
            
            i += 3; 
        } else {
            output.push_back(input[i]);
            i++;
        }
    }
    
    return output;
}

std::map<byte, unsigned> countFrequencies(IInputStream& input) {
    std::map<byte, unsigned> frequencies;
    byte value;

    while (input.Read(value)) {
        frequencies[value]++;
    }
    
    return frequencies;
}

HuffmanNode* buildHuffmanTree(const std::map<byte, unsigned>& frequencies) {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, CompareNodes> pq;
    
    for (const auto& pair : frequencies) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }
    
    if (pq.empty()) {
        return nullptr;
    }
    
    if (pq.size() == 1) {
        HuffmanNode* leaf = pq.top();
        pq.pop();
        return new HuffmanNode(leaf->frequency + 1, leaf, new HuffmanNode(0, 1));
    }
    
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top();
        pq.pop();
        
        HuffmanNode* right = pq.top();
        pq.pop();
        
        unsigned sumFreq = left->frequency + right->frequency;
        pq.push(new HuffmanNode(sumFreq, left, right));
    }
    
    return pq.top();
}

void generateCodes(HuffmanNode* node, std::string code, std::unordered_map<byte, std::string>& codes) {
    if (node == nullptr) return;
    
    if (node->isLeaf()) {
        if (code.empty()) {
            code = "0";
        }
        codes[node->value] = code;
    } else {
        generateCodes(node->left, code + "0", codes);
        generateCodes(node->right, code + "1", codes);
    }
}

void writeTree(HuffmanNode* node, BitWriter& writer) {
    if (node == nullptr) return;
    
    writer.writeBit(node->isLeaf());
    
    if (node->isLeaf()) {
        writer.writeByte(node->value);
    } else {
        writeTree(node->left, writer);
        writeTree(node->right, writer);
    }
}

HuffmanNode* readTree(BitReader& reader) {
    if (reader.isEndOfStream()) {
        return nullptr;
    }
    
    bool isLeaf = reader.readBit();
    
    if (isLeaf) {
        byte value = reader.readByte();
        return new HuffmanNode(value, 0); 
    } else {
        HuffmanNode* left = readTree(reader);
        HuffmanNode* right = readTree(reader);
        return new HuffmanNode(0, left, right);
    }
}

void freeHuffmanTree(HuffmanNode* node) {
    if (node == nullptr) return;
    
    freeHuffmanTree(node->left);
    freeHuffmanTree(node->right);
    delete node;
}

struct BWTResult {
    std::vector<byte> transformed;  
    int primaryIndex;              
};

struct BWTEntry {
    size_t index;  
    
    BWTEntry(size_t i) : index(i) {}
    
    bool operator<(const BWTEntry& other) const;
};

namespace {
    const std::vector<byte>* bwtData = nullptr;
    size_t bwtSize = 0;
}

bool BWTEntry::operator<(const BWTEntry& other) const {
    if (bwtData == nullptr || bwtSize == 0) {
        return false;
    }
    
    for (size_t i = 0; i < bwtSize; ++i) {
        size_t thisPos = (index + i) % bwtSize;
        size_t otherPos = (other.index + i) % bwtSize;
        
        if ((*bwtData)[thisPos] != (*bwtData)[otherPos]) {
            return (*bwtData)[thisPos] < (*bwtData)[otherPos];
        }
    }

    return index < other.index;
}

BWTResult bwtEncode(const std::vector<byte>& input) {
    BWTResult result;
    
    size_t size = input.size();
    if (size == 0) {
        result.primaryIndex = 0;
        return result;
    }
    
    bwtData = &input;
    bwtSize = size;
    
    std::vector<BWTEntry> entries;
    entries.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        entries.emplace_back(i);
    }

    std::sort(entries.begin(), entries.end());
    
    result.primaryIndex = -1;
    for (size_t i = 0; i < size; ++i) {
        if (entries[i].index == 0) {
            result.primaryIndex = static_cast<int>(i);
            break;
        }
    }
    
    result.transformed.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        size_t lastPos = (entries[i].index + size - 1) % size;
        result.transformed.push_back(input[lastPos]);
    }
    
    bwtData = nullptr;
    bwtSize = 0;
    
    return result;
}

std::vector<byte> bwtDecode(const std::vector<byte>& input, int primaryIndex) {
    size_t size = input.size();
    std::vector<byte> output(size);
    
    if (size == 0 || primaryIndex < 0 || primaryIndex >= static_cast<int>(size)) {
        return output;
    }

    std::vector<int> t(size);
    
    std::vector<int> freq(256, 0);
    for (byte b : input) {
        freq[b]++;
    }
    
    std::vector<int> startPos(256, 0);
    for (int i = 1; i < 256; ++i) {
        startPos[i] = startPos[i - 1] + freq[i - 1];
    }
    
    std::vector<int> posCopy = startPos;
    for (size_t i = 0; i < size; ++i) {
        byte b = input[i];
        t[i] = posCopy[b]++;
    }
    
    int pos = primaryIndex;
    std::vector<byte> tempOutput(size);
    
    for (size_t i = 0; i < size; ++i) {
        tempOutput[i] = input[pos];
        pos = t[pos];
    }
    
    for (size_t i = 0; i < size; ++i) {
        output[i] = tempOutput[size - i - 1];
    }
    
    return output;
}

void Encode(IInputStream& original, IOutputStream& compressed)
{
    BufferedInputStream bufferedInput(original);
    
    bufferedInput.bufferInput();
    
    const std::vector<byte>& originalData = bufferedInput.getData();
    if (originalData.empty()) {
        return;
    }
    
    std::vector<byte> rleInitialData = runLengthEncode(originalData);
    
    BWTResult bwtResult = bwtEncode(rleInitialData);
    
    std::vector<byte> mtfData = moveToFrontEncode(bwtResult.transformed);
    
    std::vector<byte> rleMtfData = runLengthEncode(mtfData);
    
    BufferedInputStream rleInput(original); 
    rleInput.setData(rleMtfData);
    
    std::map<byte, unsigned> frequencies = countFrequencies(rleInput);
    
    HuffmanNode* root = buildHuffmanTree(frequencies);
    if (root == nullptr) {
        return;
    }
    
    std::unordered_map<byte, std::string> codes;
    generateCodes(root, "", codes);
    
    BitWriter writer(compressed);
    
    size_t originalSize = originalData.size();
    writer.writeBits((originalSize >> 24) & 0xFF, 8);
    writer.writeBits((originalSize >> 16) & 0xFF, 8);
    writer.writeBits((originalSize >> 8) & 0xFF, 8);
    writer.writeBits(originalSize & 0xFF, 8);
    
    writer.writeBits((bwtResult.primaryIndex >> 24) & 0xFF, 8);
    writer.writeBits((bwtResult.primaryIndex >> 16) & 0xFF, 8);
    writer.writeBits((bwtResult.primaryIndex >> 8) & 0xFF, 8);
    writer.writeBits(bwtResult.primaryIndex & 0xFF, 8);
    
    size_t initialRleSize = rleInitialData.size();
    writer.writeBits((initialRleSize >> 24) & 0xFF, 8);
    writer.writeBits((initialRleSize >> 16) & 0xFF, 8);
    writer.writeBits((initialRleSize >> 8) & 0xFF, 8);
    writer.writeBits(initialRleSize & 0xFF, 8);
    
    size_t bwtSize = bwtResult.transformed.size();
    writer.writeBits((bwtSize >> 24) & 0xFF, 8);
    writer.writeBits((bwtSize >> 16) & 0xFF, 8);
    writer.writeBits((bwtSize >> 8) & 0xFF, 8);
    writer.writeBits(bwtSize & 0xFF, 8);
    
    size_t mtfSize = mtfData.size();
    writer.writeBits((mtfSize >> 24) & 0xFF, 8);
    writer.writeBits((mtfSize >> 16) & 0xFF, 8);
    writer.writeBits((mtfSize >> 8) & 0xFF, 8);
    writer.writeBits(mtfSize & 0xFF, 8);
    
    size_t finalRleSize = rleMtfData.size();
    writer.writeBits((finalRleSize >> 24) & 0xFF, 8);
    writer.writeBits((finalRleSize >> 16) & 0xFF, 8);
    writer.writeBits((finalRleSize >> 8) & 0xFF, 8);
    writer.writeBits(finalRleSize & 0xFF, 8);
    
    writeTree(root, writer);
    
    rleInput.rewind();
    byte value;
    
    while (rleInput.Read(value)) {
        const std::string& code = codes[value];
        for (char bit : code) {
            writer.writeBit(bit == '1');
        }
    }
    
    writer.flush();
    
    freeHuffmanTree(root);
}

void Decode(IInputStream& compressed, IOutputStream& original)
{
    BitReader reader(compressed);
    
    unsigned originalSize = 0;
    originalSize = (reader.readByte() << 24) | 
                   (reader.readByte() << 16) | 
                   (reader.readByte() << 8) | 
                   reader.readByte();
    
    int bwtIndex = 0;
    bwtIndex = (reader.readByte() << 24) | 
               (reader.readByte() << 16) | 
               (reader.readByte() << 8) | 
               reader.readByte();
    
    unsigned initialRleSize = 0;
    initialRleSize = (reader.readByte() << 24) | 
                     (reader.readByte() << 16) | 
                     (reader.readByte() << 8) | 
                     reader.readByte();
    
    bwtSize = (reader.readByte() << 24) | 
              (reader.readByte() << 16) | 
              (reader.readByte() << 8) | 
              reader.readByte();
    
    unsigned mtfSize = 0;
    mtfSize = (reader.readByte() << 24) | 
              (reader.readByte() << 16) | 
              (reader.readByte() << 8) | 
              reader.readByte();
    
    unsigned finalRleSize = 0;
    finalRleSize = (reader.readByte() << 24) | 
                  (reader.readByte() << 16) | 
                  (reader.readByte() << 8) | 
                  reader.readByte();
    
    if (originalSize == 0 || initialRleSize == 0 || bwtSize == 0 || mtfSize == 0 || finalRleSize == 0 || 
        bwtIndex < 0 || bwtIndex >= static_cast<int>(bwtSize)) {
        return;
    }
    
    HuffmanNode* root = readTree(reader);
    if (root == nullptr) {
        return;
    }
    
    std::vector<byte> finalRleData;
    HuffmanNode* current = root;
    
    while (finalRleData.size() < finalRleSize && !reader.isEndOfStream()) {
        bool bit = reader.readBit();
        
        if (bit) {
            current = current->right;
        } else {
            current = current->left;
        }
        
        if (current && current->isLeaf()) {
            finalRleData.push_back(current->value);
            current = root;
        }
    }

    freeHuffmanTree(root);
    
    std::vector<byte> mtfData = runLengthDecode(finalRleData);
    
    std::vector<byte> bwtData = moveToFrontDecode(mtfData);
    
    std::vector<byte> initialRleData = bwtDecode(bwtData, bwtIndex);
    
    std::vector<byte> decodedData = runLengthDecode(initialRleData);
    
    for (byte b : decodedData) {
        original.Write(b);
    }
}