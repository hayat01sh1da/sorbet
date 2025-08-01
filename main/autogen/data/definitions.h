#ifndef AUTOGEN_DEFINITIONS_H
#define AUTOGEN_DEFINITIONS_H

#include "ast/ast.h"
#include "common/common.h"

namespace sorbet::autogen {

// The types defined here are simplified views of class and constant definitions in a Ruby codebase, which we use to
// create static output information and autoloader files

// A `QualifiedName` is a vector of namerefs that includes the fully-qualified name
struct QualifiedName {
    std::vector<core::NameRef> nameParts;

    static QualifiedName fromFullName(std::vector<core::NameRef> &&fullName);

    bool empty() const {
        return nameParts.empty();
    }

    uint32_t size() const {
        return nameParts.size();
    }

    bool operator==(const QualifiedName &rhs) const {
        return nameParts == rhs.nameParts;
    }

    core::NameRef name() const {
        return nameParts.back();
    }

    std::string show(const core::GlobalState &gs) const;
    std::string join(const core::GlobalState &gs, std::string_view sep) const;
};

const uint32_t NONE_ID = (uint32_t)-1;

struct ParsedFile;
struct Reference;
struct Definition;

struct DefinitionRef;
struct ReferenceRef;

struct NamedDefinition;

enum class ClassKind { Class, Module };

// A reference to a specific `Definition` inside of a `ParsedFile`.
struct DefinitionRef {
    uint32_t _id;

    DefinitionRef() : _id(NONE_ID){};
    DefinitionRef(uint32_t id) : _id(id) {}

    uint32_t id() const {
        return _id;
    }

    bool exists() const {
        return _id != NONE_ID;
    }

    const Definition &data(const ParsedFile &pf) const;
};

// A reference to a specific `Reference` inside of a `ParsedFile`.
struct ReferenceRef {
    uint32_t _id;
    ReferenceRef() : _id(NONE_ID){};
    ReferenceRef(uint32_t id) : _id(id) {}

    uint32_t id() const {
        return _id;
    }

    bool exists() const {
        return _id != NONE_ID;
    }

    const Reference &data(const ParsedFile &pf) const;
};

// A constant definition---a class, module, constant definition, or constant alias---along with relevant metadata
struct Definition {
    enum class Type : uint8_t { Module, Class, Casgn, Alias, TypeAlias };

    // the reference to this definition. Once `AutogenWalk` is completed and a full `ParsedFile` has been created, it
    // should always be the case that
    //   definition.id.data(pf) == definition
    DefinitionRef id;

    // The symbol reference for this reference
    core::SymbolRef sym;

    // is this a class, module, constant, or alias
    Type type;
    // does this define behavior? (i.e. is it a casgn or class, not simply a namespace?)
    bool defines_behavior;
    // does it contain other things?
    bool is_empty;

    // if this is a class, then `parent_ref` will be the reference to the parent class; otherwise it will be undefined
    ReferenceRef parent_ref;

    // if this is an alias, then `aliased_ref` will be the reference it is an alias for; otherwise it will be undefined
    ReferenceRef aliased_ref;

    // which ref is the one that corresponds to this definition? I (gdritter) _believe_ that this will always be defined
    // once `AutogenWalk` has completed; please update this comment if that ever turns out to be false
    ReferenceRef defining_ref;
};
CheckSize(Definition, 24, 4);

// A `Reference` corresponds to a simple use of a constant name in a file. After a `ParsedFile` has been created, every
// constant use should have a `Reference` corresponding to it
struct Reference {
    // the reference to this reference. Once `AutogenWalk` is completed and a full `ParsedFile` has been created, it
    // should always be the case that
    //   reference.id.data(pf) == reference
    ReferenceRef id;

    // The symbol reference for this reference
    core::SymbolRef sym;

    // In which class or module was this reference used?
    DefinitionRef scope;

    // the nesting ID of this constant
    uint32_t nestingId;

    // its full qualified name
    QualifiedName name;

    // the resolved name iff we have it from Sorbet
    QualifiedName resolved;

    // loc and definitionLoc will differ if this is a defining ref, otherwise they will be the same
    core::LocOffsets loc;
    core::LocOffsets definitionLoc;

    // If this is a ref used in an `include` or `extend`, then this will point to the definition of the class in which
    // this is being `include`d or `extend`ed
    DefinitionRef parent_of;

    // `true` if this is the appearance of the constant name associated with a definition: i.e. the name of a class or
    // module or the LHS of a casgn
    bool is_defining_ref;

    // Iff this is a class, then this will be `ClassKind::Class`, otherwise `ClassKind::Module`
    ClassKind parentKind = ClassKind::Module;
};
CheckSize(Reference, 96, 8);

struct AutogenConfig {
    const std::vector<std::string> behaviorAllowedInRBIsPaths;
    const bool msgpackSkipReferenceMetadata = false;
};

// A `ParsedFile` contains all the `Definition`s and `References` used in a particular file
struct ParsedFile {
    friend class MsgpackWriter;

    // the file that was parsed
    core::FileRef file;
    // the checksum of this file
    uint32_t cksum;
    // the path on disk to this file
    std::string path;
    // nesting for every ref in this file
    std::vector<std::vector<DefinitionRef>> nestings;
    // every statically-known constant defined by this file
    std::vector<Definition> defs;
    // every static constant usage in this file
    std::vector<Reference> refs;
    // every required gem in this file
    std::vector<core::NameRef> requireStatements;

    std::string toString(const core::GlobalState &gs, int version) const;
    std::string toMsgpack(core::Context ctx, int version, const AutogenConfig &autogenCfg);
    static std::string msgpackGlobalHeader(int version, size_t numFiles, const AutogenConfig &autogenCfg);

    std::vector<core::NameRef> showFullName(const core::GlobalState &gs, DefinitionRef id) const;
    QualifiedName showQualifiedName(const core::GlobalState &gs, DefinitionRef id) const;
    std::vector<std::string> listAllClasses(core::Context ctx);
};

} // namespace sorbet::autogen
#endif // AUTOGEN_DEFINITIONS_H
