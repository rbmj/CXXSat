struct Circuit::impl {
    /*
    void reg(Input* i) {
        inputs.insert(i);
    }
    void unreg(Input* i) {
        inputs.erase(inputs.find(i));
    }
    */
    /*
    void reg(Value* v) {
        outputs.insert(v);
    }
    void unreg(Value* v) {
        outputs.erase(outputs.find(v));
    }
    */
    void reg(Gate* g) {
        gates.insert(g);
    }
    void unreg(Gate* g) {
        gates.erase(gates.find(g));
    }
    void reg(Wire* w) {
        wires.insert(w);
    }
    void unreg(Wire* w) {
        wires.erase(wires.find(w));
    }
    std::shared_ptr<Input> lit0;
    std::shared_ptr<Input> lit1;
    //std::unordered_set<Input*> inputs;
    //std::unordered_set<Value*> outputs;
    std::unordered_set<Gate*> gates;
    std::unordered_set<Wire*> wires;
    std::weak_ptr<Circuit::impl> self;
    void number();
    Problem generateCNF();
};

