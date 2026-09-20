#ifndef UTAFILEBASE_H
#define UTAFILEBASE_H

#include <map>
#include <vector>
#include <filesystem>
#include <iostream>

#include <stdutau/utaglobal.h>

namespace Utau {

    /// Base of the UTAU file formats this library reads and writes.
    ///
    /// A subclass says how one format is laid out, in read() and write(). load() and save() add
    /// nothing but the file to put it in.
    ///
    /// None of these formats records its own encoding, so what a subclass holds is raw bytes.
    class STDUTAU_EXPORT UtaFileBase {
    public:
        UtaFileBase();
        virtual ~UtaFileBase();

        /// Opens \a path and hands it to read(). Returns \c false when the file will not open.
        bool load(const std::filesystem::path &path);

        /// Creates \a path and hands it to write(). Returns \c false when the file will not open.
        bool save(const std::filesystem::path &path) const;

        /// Reads the whole stream, returns whether it was understood.
        ///
        /// \note Read the lines with readLine() rather than \c std::getline . UTAU writes CRLF,
        ///       which text mode strips on Windows and nowhere else.
        virtual bool read(std::istream &is) = 0;

        /// Writes the whole contents, returns whether the stream stayed good throughout.
        virtual bool write(std::ostream &os) const = 0;
    };

}

#endif // UTAFILEBASE_H
