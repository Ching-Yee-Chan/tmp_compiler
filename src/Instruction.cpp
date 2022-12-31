#include "Instruction.h"
#include "BasicBlock.h"
#include <iostream>
#include <cassert>
#include "Function.h"
#include "Type.h"
extern FILE* yyout;

//bool assignFuncCallResult = false;

Instruction::Instruction(unsigned instType, BasicBlock *insert_bb)
{
    prev = next = this;
    opcode = -1;
    this->instType = instType;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

Instruction::~Instruction()
{
    parent->remove(this);
}

BasicBlock *Instruction::getParent()
{
    return parent;
}

void Instruction::setParent(BasicBlock *bb)
{
    parent = bb;
}

void Instruction::setNext(Instruction *inst)
{
    next = inst;
}

void Instruction::setPrev(Instruction *inst)
{
    prev = inst;
}

Instruction *Instruction::getNext()
{
    return next;
}

Instruction *Instruction::getPrev()
{
    return prev;
}

BinaryInstruction::BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb) : Instruction(BINARY, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

BinaryInstruction::~BinaryInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void BinaryInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[0]->getType()->toStr();
    switch (opcode)
    {
    case ADD:
        op = "add";
        break;
    case SUB:
        op = "sub";
        break;
    case MUL:
        op = "mul";
        break;
    case DIV:
        op = "sdiv";
        break;
    case MOD:
        op = "srem";
        break;
    default:
        break;
    }
    fprintf(yyout, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

CmpInstruction::CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb): Instruction(CMP, insert_bb){
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

CmpInstruction::~CmpInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void CmpInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[1]->getType()->toStr();
    switch (opcode)
    {
    case E:
        op = "eq";
        break;
    case NE:
        op = "ne";
        break;
    case L:
        op = "slt";
        break;
    case LE:
        op = "sle";
        break;
    case G:
        op = "sgt";
        break;
    case GE:
        op = "sge";
        break;
    default:
        op = "";
        break;
    }

    fprintf(yyout, "  %s = icmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

UncondBrInstruction::UncondBrInstruction(BasicBlock *to, BasicBlock *insert_bb) : Instruction(UNCOND, insert_bb)
{
    branch = to;
}

void UncondBrInstruction::output() const
{
    fprintf(yyout, "  br label %%B%d\n", branch->getNo());
}

void UncondBrInstruction::setBranch(BasicBlock *bb)
{
    branch = bb;
}

BasicBlock *UncondBrInstruction::getBranch()
{
    return branch;
}

CondBrInstruction::CondBrInstruction(BasicBlock*true_branch, BasicBlock*false_branch, Operand *cond, BasicBlock *insert_bb) : Instruction(COND, insert_bb){
    this->true_branch = true_branch;
    this->false_branch = false_branch;
    cond->addUse(this);
    operands.push_back(cond);
}

CondBrInstruction::~CondBrInstruction()
{
    operands[0]->removeUse(this);
}

void CondBrInstruction::output() const
{
    std::string cond, type;
    cond = operands[0]->toStr();
    type = operands[0]->getType()->toStr();
    int true_label = true_branch->getNo();
    int false_label = false_branch->getNo();
    fprintf(yyout, "  br %s %s, label %%B%d, label %%B%d\n", type.c_str(), cond.c_str(), true_label, false_label);
}

void CondBrInstruction::setFalseBranch(BasicBlock *bb)
{
    false_branch = bb;
}

BasicBlock *CondBrInstruction::getFalseBranch()
{
    return false_branch;
}

void CondBrInstruction::setTrueBranch(BasicBlock *bb)
{
    true_branch = bb;
}

BasicBlock *CondBrInstruction::getTrueBranch()
{
    return true_branch;
}

RetInstruction::RetInstruction(Operand *src, BasicBlock *insert_bb) : Instruction(RET, insert_bb)
{
    if(src != nullptr)
    {
        operands.push_back(src);
        src->addUse(this);
    }
}

RetInstruction::~RetInstruction()
{
    if(!operands.empty())
        operands[0]->removeUse(this);
}

void RetInstruction::output() const
{
    if(operands.empty())
    {
        fprintf(yyout, "  ret void\n");
    }
    else
    {
        std::string ret, type;
        ret = operands[0]->toStr();
        type = operands[0]->getType()->toStr();
        fprintf(yyout, "  ret %s %s\n", type.c_str(), ret.c_str());
    }
}

AllocaInstruction::AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb) : Instruction(ALLOCA, insert_bb)
{
    operands.push_back(dst);
    dst->setDef(this);
    this->se = se;
}

AllocaInstruction::~AllocaInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
}

void AllocaInstruction::output() const
{
    std::string dst, type;
    dst = operands[0]->toStr();
    type = se->getType()->toStr();
    fprintf(yyout, "  %s = alloca %s, align 4\n", dst.c_str(), type.c_str());
}

LoadInstruction::LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb) : Instruction(LOAD, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src_addr);
    dst->setDef(this);
    src_addr->addUse(this);
}

LoadInstruction::~LoadInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void LoadInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string src_type;
    std::string dst_type;
    dst_type = operands[0]->getType()->toStr();
    src_type = operands[1]->getType()->toStr();
    fprintf(yyout, "  %s = load %s, %s %s, align 4\n", dst.c_str(), dst_type.c_str(), src_type.c_str(), src.c_str());
}

CallInstruction::CallInstruction(Operand *dst, std::vector<Operand*> params, IdentifierSymbolEntry* funcse, BasicBlock *insert_bb) : Instruction(CALL, insert_bb)
{
    operands.push_back(dst);//返回值，目标寄存器
    dst->setDef(this);
    for(auto param : params){//参数，源寄存器
        operands.push_back(param);
        param->addUse(this);
    }
    funcSE = funcse;
}

CallInstruction::~CallInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    for(int i = 1;i<(int)operands.size(); i++){
        operands[i]->removeUse(this);
    }
}
//只输出call语句前半部分：call void @putch(
void CallInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string dst_type;
    dst_type = operands[0]->getType()->toStr();
    Type* returnType = dynamic_cast<FunctionType*>(funcSE->getType())->getRetType();
    if(!returnType->isVoid()){//仅当返回值为非void时，向临时寄存器赋值
        fprintf(yyout, "  %s =", dst.c_str());
    }
    fprintf(yyout, "  call %s %s(", dst_type.c_str(), funcSE->toStr().c_str());
    for(int i = 1;i<(int)operands.size(); i++){
        std::string src = operands[i]->toStr();
        std::string src_type = operands[i]->getType()->toStr();
        if(i!=1){
            fprintf(yyout, ", ");
        }
        fprintf(yyout, "%s %s", src_type.c_str(), src.c_str());
    }
    fprintf(yyout, ")\n");
}

StoreInstruction::StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb) : Instruction(STORE, insert_bb)
{
    operands.push_back(dst_addr);
    operands.push_back(src);
    dst_addr->addUse(this);
    src->addUse(this);
}

StoreInstruction::~StoreInstruction()
{
    operands[0]->removeUse(this);
    operands[1]->removeUse(this);
}

void StoreInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();

    fprintf(yyout, "  store %s %s, %s %s, align 4\n", src_type.c_str(), src.c_str(), dst_type.c_str(), dst.c_str());
}

ZextInstruction::ZextInstruction(Operand *src, Operand *dst, BasicBlock *insert_bb) : Instruction(ZEXT, insert_bb)
{
    operands.push_back(src);
    operands.push_back(dst);
    dst->setDef(this);
    src->addUse(this);
}

ZextInstruction::~ZextInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void ZextInstruction::output() const
{
    std::string src = operands[0]->toStr();
    std::string dst = operands[1]->toStr();
    std::string src_type = operands[0]->getType()->toStr();
    std::string dst_type = operands[1]->getType()->toStr();
    fprintf(yyout, "  %s = zext %s %s to %s\n", dst.c_str(), src_type.c_str(), src.c_str(), dst_type.c_str());
}

FBinaryInstruction::FBinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb) : Instruction(FBINARY, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

FBinaryInstruction::~FBinaryInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void FBinaryInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[0]->getType()->toStr();
    switch (opcode)
    {
    case ADD:
        op = "fadd";
        break;
    case SUB:
        op = "fsub";
        break;
    case MUL:
        op = "fmul";
        break;
    case DIV:
        op = "fdiv";
        break;
    default:
        break;
    }
    fprintf(yyout, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

FCmpInstruction::FCmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb) : Instruction(FCMP, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

FCmpInstruction::~FCmpInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void FCmpInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[1]->getType()->toStr();
    switch (opcode)
    {
    case E:
        op = "oeq";
        break;
    case NE:
        op = "one";
        break;
    case L:
        op = "olt";
        break;
    case LE:
        op = "ole";
        break;
    case G:
        op = "ogt";
        break;
    case GE:
        op = "oge";
        break;
    default:
        op = "";
        break;
    }

    fprintf(yyout, "  %s = fcmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

IntFloatCastInstructionn::IntFloatCastInstructionn(unsigned opcode, Operand *src, Operand *dst, BasicBlock *insert_bb) : Instruction(CAST, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(src);
    operands.push_back(dst);
    dst->setDef(this);
    src->addUse(this);
}

IntFloatCastInstructionn::~IntFloatCastInstructionn()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void IntFloatCastInstructionn::output() const
{
    std::string src = operands[0]->toStr();
    std::string dst = operands[1]->toStr();
    std::string src_type = operands[0]->getType()->toStr();
    std::string dst_type = operands[1]->getType()->toStr();
    std::string castType;
    switch(opcode) {
        case I2F:
            castType = "sitofp";
            break;
        case F2I:
            castType = "fptosi";
            break;
        default:
            castType = "";
            break;
    }
    fprintf(yyout, "  %s = %s %s %s to %s\n", dst.c_str(), castType.c_str(), src_type.c_str(), src.c_str(), dst_type.c_str());
}
//lab7 starts here
MachineOperand* Instruction::genMachineOperand(Operand* ope)
{
    auto se = ope->getEntry();
    MachineOperand* mope = nullptr;
    if(se->isConstant())
        mope = new MachineOperand(MachineOperand::IMM, dynamic_cast<ConstantSymbolEntry*>(se)->getValue());
    else if(se->isTemporary())
        mope = new MachineOperand(MachineOperand::VREG, dynamic_cast<TemporarySymbolEntry*>(se)->getLabel());
    else if(se->isVariable())
    {
        auto id_se = dynamic_cast<IdentifierSymbolEntry*>(se);
        Type* type = id_se->getType();
        // 如果是函数参数
        if(id_se->isParam()) {
            auto param_id = this->parent->getParent()->getParamId(ope);
            if(param_id >= 0 && param_id <= 3) {
                mope = new MachineOperand(MachineOperand::REG, param_id);
            }
            else if(param_id >= 4) {

            }
        }
        // 如果是全局变量
        else if(id_se->isGlobal()) {
            mope = new MachineOperand(id_se->toStr().erase(0,1).c_str());
        }
        else {
            if(type==TypeSystem::constIntType){
                //TODO: add array manipulation here
                //TODO: add float manipulation here
                mope = new MachineOperand(MachineOperand::IMM, id_se->value);
            }
            else{
                mope = new MachineOperand(id_se->toStr().c_str());
            }
        }
    }
    return mope;
}

MachineOperand* Instruction::genMachineReg(int reg) 
{
    return new MachineOperand(MachineOperand::REG, reg);
}

MachineOperand* Instruction::genMachineVReg() 
{
    return new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
}

MachineOperand* Instruction::genMachineImm(int val) 
{
    return new MachineOperand(MachineOperand::IMM, val);
}

MachineOperand* Instruction::genMachineLabel(int block_no)
{
    std::ostringstream buf;
    buf << ".L" << block_no;
    std::string label = buf.str();
    return new MachineOperand(label);
}

void AllocaInstruction::genMachineCode(AsmBuilder* builder)
{
    /* HINT:
    * Allocate stack space for local variabel
    * Store frame offset in symbol entry */
    auto cur_func = builder->getFunction();
    int offset = cur_func->AllocSpace(4);
    dynamic_cast<TemporarySymbolEntry*>(operands[0]->getEntry())->setOffset(-offset);
}

void LoadInstruction::genMachineCode(AsmBuilder* builder)
{
    auto cur_block = builder->getBlock();
    MachineInstruction* cur_inst = nullptr;
    // Load global operand
    if(operands[1]->getEntry()->isVariable()
    && dynamic_cast<IdentifierSymbolEntry*>(operands[1]->getEntry())->isGlobal())
    {
        auto dst = genMachineOperand(operands[0]);
        auto internal_reg1 = genMachineVReg();
        auto internal_reg2 = new MachineOperand(*internal_reg1);
        auto src = genMachineOperand(operands[1]);
        // example: load r0, addr_a
        cur_inst = new LoadMInstruction(cur_block, internal_reg1, src);
        cur_block->InsertInst(cur_inst);
        // example: load r1, [r0]
        cur_inst = new LoadMInstruction(cur_block, dst, internal_reg2);
        cur_block->InsertInst(cur_inst);
    }
    // Load local operand
    else if(operands[1]->getEntry()->isTemporary()
    && operands[1]->getDef()
    && operands[1]->getDef()->isAlloc())
    {
        // example: load r1, [r0, #4]
        auto dst = genMachineOperand(operands[0]);
        auto src1 = genMachineReg(11);
        auto src2 = genMachineImm(dynamic_cast<TemporarySymbolEntry*>(operands[1]->getEntry())->getOffset());
        cur_inst = new LoadMInstruction(cur_block, dst, src1, src2);
        cur_block->InsertInst(cur_inst);
        // 如果是函数参数 则保留其偏移量方便后续调整
        // 如果偏移量为正值则说明其在栈底之上 为保留参数
        if(dynamic_cast<TemporarySymbolEntry*>(operands[1]->getEntry())->getOffset() >= 0) {
            cur_block->getParent()->insertSavedParamsOffset(src2);
        }
    }
    // Load operand from temporary variable
    else
    {
        // example: load r1, [r0]
        auto dst = genMachineOperand(operands[0]);
        auto src = genMachineOperand(operands[1]);
        cur_inst = new LoadMInstruction(cur_block, dst, src);
        cur_block->InsertInst(cur_inst);
    }
}

void StoreInstruction::genMachineCode(AsmBuilder* builder)
{
    auto cur_block = builder->getBlock();
    MachineInstruction* cur_inst = nullptr;
    // 如果当前是存参数 则直接修改dst的offset
    if(operands[1]->getEntry()->isVariable()) {
        auto id_se = dynamic_cast<IdentifierSymbolEntry*>(operands[1]->getEntry());
        if(id_se->isParam()) {
            int param_id = this->getParent()->getParent()->getParamId(operands[1]);
            if(param_id >= 4) {
                int offset = 4 * (param_id - 4);
                dynamic_cast<TemporarySymbolEntry*>(operands[0]->getEntry())->setOffset(offset);
                return;
            }
        }
    }
    MachineOperand* src = genMachineOperand(operands[1]);
    //如果src为常数，需要先load进来
    if(src->isImm()){
        auto internal_reg = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, internal_reg, src);
        cur_block->InsertInst(cur_inst);
        src = new MachineOperand(*internal_reg);
    }
    // store global operand
    if(operands[0]->getEntry()->isVariable()
    && dynamic_cast<IdentifierSymbolEntry*>(operands[0]->getEntry())->isGlobal())
    {
        auto internal_reg1 = genMachineVReg();
        auto internal_reg2 = new MachineOperand(*internal_reg1);
        auto dst = genMachineOperand(operands[0]);
        // example: load r0, addr_a
        cur_inst = new LoadMInstruction(cur_block, internal_reg1, dst);
        cur_block->InsertInst(cur_inst);
        // example: store r1, [r0]
        cur_inst = new StoreMInstruction(cur_block, src, internal_reg2);
        cur_block->InsertInst(cur_inst);
    }
    // store local operand
    else if(operands[0]->getEntry()->isTemporary()
    && operands[0]->getDef()
    && operands[0]->getDef()->isAlloc())
    {
        // example: store r1, [r0, #4]
        auto dst1 = genMachineReg(11);
        auto dst2 = genMachineImm(dynamic_cast<TemporarySymbolEntry*>(operands[0]->getEntry())->getOffset());
        cur_inst = new StoreMInstruction(cur_block, src, dst1, dst2);
        cur_block->InsertInst(cur_inst);
    }
    // store operand from temporary variable
    else
    {
        // example: store r1, [r0]
        auto dst = genMachineOperand(operands[0]);
        cur_inst = new StoreMInstruction(cur_block, src, dst);
        cur_block->InsertInst(cur_inst);
    }
}

void BinaryInstruction::genMachineCode(AsmBuilder* builder)
{
    // TODO: 超过范围的立即数
    // complete other instructions
    auto cur_block = builder->getBlock();
    auto dst = genMachineOperand(operands[0]);
    auto src1 = genMachineOperand(operands[1]);
    auto src2 = genMachineOperand(operands[2]);
    /* HINT:
    * The source operands of ADD instruction in ir code both can be immediate num.
    * However, it's not allowed in assembly code.
    * So you need to insert LOAD/MOV instrucrion to load immediate num into register.
    * As to other instructions, such as MUL, CMP, you need to deal with this situation, too.*/
    MachineInstruction* cur_inst = nullptr;
    if(src1->isImm())
    {
        auto internal_reg = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, internal_reg, src1);
        cur_block->InsertInst(cur_inst);
        src1 = new MachineOperand(*internal_reg);
    }
    if(opcode == MUL || opcode == DIV || opcode == MOD)
    {
        if(src2->isImm())
        {
            auto internal_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block, internal_reg, src2);
            cur_block->InsertInst(cur_inst);
            src2 = new MachineOperand(*internal_reg);
        }
    }
    {
        if(src2->isImm())
        {
            auto internal_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block, internal_reg, src2);
            cur_block->InsertInst(cur_inst);
            src2 = new MachineOperand(*internal_reg);
        }
    }
    switch (opcode)
    {
    case ADD:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, dst, src1, src2);
        break;
    case SUB:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::SUB, dst, src1, src2);
        break;
    case MUL:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, dst, src1, src2);
        break;
    case DIV:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::DIV, dst, src1, src2);
        break;
    case MOD: {
        // a % b = a - a / b * b
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::DIV, dst, src1, src2);
        MachineOperand *dst1 = new MachineOperand(*dst);
        src1 = new MachineOperand(*src1);
        src2 = new MachineOperand(*src2);
        cur_block->InsertInst(cur_inst);
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, dst1, dst, src2);
        cur_block->InsertInst(cur_inst);
        dst = new MachineOperand(*dst1);
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::SUB, dst, src1, dst1);
        break;
    }
    case AND:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::AND, dst, src1, src2);
        break;
    case OR:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::OR, dst, src1, src2);
        break;
    default:
        break;
    }
    cur_block->InsertInst(cur_inst);
}

void CmpInstruction::genMachineCode(AsmBuilder* builder)
{
    MachineBlock* cur_block = builder->getBlock();
    MachineOperand* src1 = genMachineOperand(operands[1]);
    MachineOperand* src2 = genMachineOperand(operands[2]);
    MachineInstruction* cur_inst = nullptr;
    if (src1->isImm()) {
        MachineOperand* internal_reg = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, internal_reg, src1);
        cur_block->InsertInst(cur_inst);
        src1 = new MachineOperand(*internal_reg);
    }
    if (src2->isImm()) {
        MachineOperand* internal_reg = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, internal_reg, src2);
        cur_block->InsertInst(cur_inst);
        src2 = new MachineOperand(*internal_reg);
    }
    cur_inst = new CmpMInstruction(cur_block, src1, src2, opcode);
    cur_block->InsertInst(cur_inst);
    cur_block->setCurrentBranchCond(opcode);
    // 采用条件存储的方式将1/0存储到dst中
    MachineOperand* dst = genMachineOperand(operands[0]);
    MachineOperand* trueOperand = genMachineImm(1);
    MachineOperand* falseOperand = genMachineImm(0);
    cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, trueOperand, opcode);
    cur_block->InsertInst(cur_inst);
    if(opcode == CmpInstruction::E || opcode == CmpInstruction::NE){
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, falseOperand, 1-opcode);
    }
    else {
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, falseOperand, 7-opcode);
    }
    cur_block->InsertInst(cur_inst);
}

void UncondBrInstruction::genMachineCode(AsmBuilder* builder)
{
    MachineBlock* cur_block = builder->getBlock();
    std::stringstream label;
    label << ".L" << branch->getNo();
    MachineOperand* dst = new MachineOperand(label.str());
    MachineInstruction* cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, dst);
    cur_block->InsertInst(cur_inst);
}

void CondBrInstruction::genMachineCode(AsmBuilder* builder)
{
    MachineBlock* cur_block = builder->getBlock();
    std::stringstream true_label, false_label;
    true_label << ".L" << true_branch->getNo();
    false_label << ".L" << false_branch->getNo();
    MachineOperand* true_dst = new MachineOperand(true_label.str());
    MachineOperand* false_dst = new MachineOperand(false_label.str());
    // 符合当前块跳转条件有条件跳转到真分支
    MachineInstruction* cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, true_dst, cur_block->getCurrentBranchCond());
    cur_block->InsertInst(cur_inst);
    // 不符合当前块跳转条件无条件跳转到假分支
    cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, false_dst);
    cur_block->InsertInst(cur_inst);
}

void RetInstruction::genMachineCode(AsmBuilder* builder)
{
    auto cur_block = builder->getBlock();
    MachineInstruction* cur_inst = nullptr;
    //1. Generate mov instruction to save return value in r0
    if(!operands.empty()){
        auto src = genMachineOperand(operands[0]);
        //立即数->寄存器
        if(src->isImm())
        {
            auto internal_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block, internal_reg, src);
            cur_block->InsertInst(cur_inst);
            src = new MachineOperand(*internal_reg);
        }
        auto dst = new MachineOperand(MachineOperand::REG, 0);//r0
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, src);
        cur_block->InsertInst(cur_inst);
    }
    // 生成一条跳转到结尾函数栈帧处理的无条件跳转语句
    auto dst = new MachineOperand(".L" + this->getParent()->getParent()->getSymPtr()->toStr().erase(0,1) + "_END");
    cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, dst);
    cur_block->InsertInst(cur_inst);
    //接下来的工作放到MachineCode.cpp: void MachineFunction::output()完成
}

void CallInstruction::genMachineCode(AsmBuilder* builder)
{
    auto cur_block = builder->getBlock();
    MachineInstruction* cur_inst = nullptr;
    std::vector<MachineOperand*> additional_args;
    for(unsigned int i = 1;i < operands.size();i++){
        //左起前4个参数通过r0-r3传递
        if(i<=4){
            auto dst = new MachineOperand(MachineOperand::REG, i-1);//r0-r3
            cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, genMachineOperand(operands[i]));
            cur_block->InsertInst(cur_inst);
        }
        else{
            additional_args.push_back(genMachineOperand(operands[i]));
        }
    }
    if(!additional_args.empty()){
        // 参数需要逆序压栈
        std::reverse(additional_args.begin(), additional_args.end());
        cur_inst = new StackMInstruction(cur_block, StackMInstruction::PUSH, additional_args);
        cur_block->InsertInst(cur_inst);
    }
    cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::BL, new MachineOperand(funcSE->getName(), true));
    cur_block->InsertInst(cur_inst);
    // 对于有返回值的函数调用 需要提供一条从mov r0, dst的指令
    if(dynamic_cast<FunctionType*>(this->funcSE->getType())->getRetType() != TypeSystem::voidType) {
        auto dst = genMachineOperand(operands[0]);
        auto src = new MachineOperand(MachineOperand::REG, 0);//r0
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, src);
        cur_block->InsertInst(cur_inst);
    }
    // 恢复栈帧 调整sp
    if(!additional_args.empty()){
        auto src1 = genMachineReg(13);
        auto src2 = genMachineImm(additional_args.size()*4);
        auto dst = genMachineReg(13);
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, dst, src1, src2);
        cur_block->InsertInst(cur_inst);
    }
}


void ZextInstruction::genMachineCode(AsmBuilder* builder)
{
    MachineBlock* cur_block = builder->getBlock();
    MachineInstruction* cur_inst = nullptr;
    MachineOperand* src = genMachineOperand(operands[0]);
    if(src->isImm())
    {
        auto internal_reg = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, internal_reg, src);
        cur_block->InsertInst(cur_inst);
        src = new MachineOperand(*internal_reg);
    }
    MachineOperand* dst = genMachineOperand(operands[1]);
    cur_inst = new ZextMInstruction(cur_block, dst, src);
    cur_block->InsertInst(cur_inst);
}

void FBinaryInstruction::genMachineCode(AsmBuilder* builder)
{
    // TODO
}

void FCmpInstruction::genMachineCode(AsmBuilder* builder)
{
    // TODO
}

void IntFloatCastInstructionn::genMachineCode(AsmBuilder* builder)
{
    // TODO
}