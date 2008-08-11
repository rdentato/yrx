/the quick brown fox/
    the quick brown fox
    The quick brown FOX
    What do you know about the quick brown fox?
    What do you know about THE QUICK BROWN FOX?

/\CThe quick brown fox/
    the quick brown fox
    The quick brown FOX
    What do you know about the quick brown fox?
    What do you know about THE QUICK BROWN FOX?

/abcd\t\n\r\f\a\x1B\071\x3b\$\\\?caxyz/
    abcd\t\n\r\f\a\x1B9;\$\\?caxyz

/a*abc?xyz+pqr{3}ab{2,}xy{4,5}pq{0,6}AB{0,}zz/
    abxyzpqrrrabbxyyyypqAzz
    abxyzpqrrrabbxyyyypqAzz
    aabxyzpqrrrabbxyyyypqAzz
    aaabxyzpqrrrabbxyyyypqAzz
    aaaabxyzpqrrrabbxyyyypqAzz
    abcxyzpqrrrabbxyyyypqAzz
    aabcxyzpqrrrabbxyyyypqAzz
    aaabcxyzpqrrrabbxyyyypAzz
    aaabcxyzpqrrrabbxyyyypqAzz
    aaabcxyzpqrrrabbxyyyypqqAzz
    aaabcxyzpqrrrabbxyyyypqqqAzz
    aaabcxyzpqrrrabbxyyyypqqqqAzz
    aaabcxyzpqrrrabbxyyyypqqqqqAzz
    aaabcxyzpqrrrabbxyyyypqqqqqqAzz
    aaaabcxyzpqrrrabbxyyyypqAzz
    abxyzzpqrrrabbxyyyypqAzz
    aabxyzzzpqrrrabbxyyyypqAzz
    aaabxyzzzzpqrrrabbxyyyypqAzz
    aaaabxyzzzzpqrrrabbxyyyypqAzz
    abcxyzzpqrrrabbxyyyypqAzz
    aabcxyzzzpqrrrabbxyyyypqAzz
    aaabcxyzzzzpqrrrabbxyyyypqAzz
    aaaabcxyzzzzpqrrrabbxyyyypqAzz
    aaaabcxyzzzzpqrrrabbbxyyyypqAzz
    aaaabcxyzzzzpqrrrabbbxyyyyypqAzz
    aaabcxyzpqrrrabbxyyyypABzz
    aaabcxyzpqrrrabbxyyyypABBzz
    >>>aaabxyzpqrrrabbxyyyypqAzz
    >aaaabxyzpqrrrabbxyyyypqAzz
    >>>>abcxyzpqrrrabbxyyyypqAzz
    abxyzpqrrabbxyyyypqAzz
    abxyzpqrrrrabbxyyyypqAzz
    abxyzpqrrrabxyyyypqAzz
    aaaabcxyzzzzpqrrrabbbxyyyyyypqAzz
    aaaabcxyzzzzpqrrrabbbxyyypqAzz
    aaabcxyzpqrrrabbxyyyypqqqqqqqAzz

/^(abc){1,2}zz/
    abczz
    abcabczz
    zz
    abcabcabczz
    >>abczz

/^(b+?|a){1,2}?c/
    bc
    bbc
    bbbc
    bac
    bbac
    aac
    abbbbbbbbbbbc
    bbbbbbbbbbbac
    aaac
    abbbbbbbbbbbac

/^(b+|a){1,2}c/
    bc
    bbc
    bbbc
    bac
    bbac
    aac
    abbbbbbbbbbbc
    bbbbbbbbbbbac
    aaac
    abbbbbbbbbbbac

/^(b+|a){1,2}?bc/
    bbc

/^(b*|ba){1,2}?bc/
    babc
    bbabc
    bababc
    bababbc
    babababc

/^(ba|b*){1,2}?bc/
    babc
    bbabc
    bababc
    bababbc
    babababc

/^\ca\cA\c[\c{\c:/
    \x01\x01\x1B;z

/^[ab\]cde]/
    athing
    bthing
    ]thing
    cthing
    dthing
    ething
    fthing
    [thing
    \\thing

/^[]cde]/
    ]thing
    cthing
    dthing
    ething
    athing
    fthing

/^[^ab\]cde]/
    fthing
    [thing
    \\thing
    athing
    bthing
    ]thing
    cthing
    dthing
    ething

/^[^]cde]/
    athing
    fthing
    ]thing
    cthing
    dthing
    ething

/^\�/
    �

/^�/
    �

/^[0-9]+$/
    0
    1
    2
    3
    4
    5
    6
    7
    8
    9
    10
    100
    abc

/^.*nter/
    enter
    inter
    uponter

/^xxx[0-9]+$/
    xxx0
    xxx1234
    xxx

/^.+[0-9][0-9][0-9]$/
    x123
    xx123
    123456
    123
    x1234

/^.+?[0-9][0-9][0-9]$/
    x123
    xx123
    123456
    123
    x1234

/^([^!]+)!(.+)=apquxz\.ixr\.zzz\.ac\.uk$/
    abc!pqr=apquxz.ixr.zzz.ac.uk
    !pqr=apquxz.ixr.zzz.ac.uk
    abc!=apquxz.ixr.zzz.ac.uk
    abc!pqr=apquxz:ixr.zzz.ac.uk
    abc!pqr=apquxz.ixr.zzz.ac.ukk

/:/
    Well, we need a colon: somewhere
    Fail if we don't

/([\da-f:]+)$/i
    0abc
    abc
    fed
    E
    ::
    5f03:12C0::932e
    fed def
    Any old stuff
    0zzz
    gzzz
    fed\x20
    Any old rubbish

/^.*\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/
    .1.2.3
    A.12.123.0
    .1.2.3333
    1.2.3
    1234.2.3

/^(\d+)\s+IN\s+SOA\s+(\S+)\s+(\S+)\s*\(\s*$/
    1 IN SOA non-sp1 non-sp2(
    1    IN    SOA    non-sp1    non-sp2   (
    1IN SOA non-sp1 non-sp2(

/^[a-zA-Z\d][a-zA-Z\d\-]*(\.[a-zA-Z\d][a-zA-z\d\-]*)*\.$/
    a.
    Z.
    2.
    ab-c.pq-r.
    sxk.zzz.ac.uk.
    x-.y-.
    -abc.peq.

/^\*\.[a-z]([a-z\-\d]*[a-z\d]+)?(\.[a-z]([a-z\-\d]*[a-z\d]+)?)*$/
    *.a
    *.b0-a
    *.c3-b.c
    *.c-a.b-c
    *.0
    *.a-
    *.a-b.c-
    *.c-a.0-c

/^(?=ab(de))(abd)(e)/
    abde

/^(?!(ab)de|x)(abd)(f)/
    abdf

/^(?=(ab(cd)))(ab)/
    abcd

/^[\da-f](\.[\da-f])*$/i
    a.b.c.d
    A.B.C.D
    a.b.c.1.2.3.C

/^\".*\"\s*(;.*)?$/
    \"1234\"
    \"abcd\" ;
    \"\" ; rhubarb
    \"1234\" : things

/^$/
    \

/   ^    a   (?# begins with a)  b\sc (?# then b c) $ (?# then end)/x
    ab c
    abc
    ab cde

/(?x)   ^    a   (?# begins with a)  b\sc (?# then b c) $ (?# then end)/
    ab c
    abc
    ab cde

/^   a\ b[c ]d       $/x
    a bcd
    a b d
    abcd
    ab d

/^(a(b(c)))(d(e(f)))(h(i(j)))(k(l(m)))$/
    abcdefhijklm

/^(?:a(b(c)))(?:d(e(f)))(?:h(i(j)))(?:k(l(m)))$/
    abcdefhijklm

/^[\w][\W][\s][\S][\d][\D][\b][\n][\c]][\022]/
    a+ Z0+\x08\n\x1d\x12

/^[.^$|()*+?{,}]+/
    .^\$(*+)|{?,?}

/^a*\w/
    z
    az
    aaaz
    a
    aa
    aaaa
    a+
    aa+

/^a*?\w/
    z
    az
    aaaz
    a
    aa
    aaaa
    a+
    aa+

/^a+\w/
    az
    aaaz
    aa
    aaaa
    aa+

/^a+?\w/
    az
    aaaz
    aa
    aaaa
    aa+

/^\d{8}\w{2,}/
    1234567890
    12345678ab
    12345678__
    1234567

/^[aeiou\d]{4,5}$/
    uoie
    1234
    12345
    aaaaa
    123456

/^[aeiou\d]{4,5}?/
    uoie
    1234
    12345
    aaaaa
    123456

/\A(abc|def)=(\1){2,3}\Z/
    abc=abcabc
    def=defdefdef
    abc=defdef

/^(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)\11*(\3\4)\1(?#)2$/
    abcdefghijkcda2
    abcdefghijkkkkcda2

/(cat(a(ract|tonic)|erpillar)) \1()2(3)/
    cataract cataract23
    catatonic catatonic23
    caterpillar caterpillar23


/^From +([^ ]+) +[a-zA-Z][a-zA-Z][a-zA-Z] +[a-zA-Z][a-zA-Z][a-zA-Z] +[0-9]?[0-9] +[0-9][0-9]:[0-9][0-9]/
    From abcd  Mon Sep 01 12:33:02 1997

/^From\s+\S+\s+([a-zA-Z]{3}\s+){2}\d{1,2}\s+\d\d:\d\d/
    From abcd  Mon Sep 01 12:33:02 1997
    From abcd  Mon Sep  1 12:33:02 1997
    From abcd  Sep 01 12:33:02 1997

/^12.34/s
    12\n34
    12\r34

/\w+(?=\t)/
    the quick brown\t fox

/foo(?!bar)(.*)/
    foobar is foolish see?

/(?:(?!foo)...|^.{0,2})bar(.*)/
    foobar crowbar etc
    barrel
    2barrel
    A barrel

/^(\D*)(?=\d)(?!123)/
    abc456
    abc123

/^1234(?# test newlines
  inside)/
    1234

/^1234 #comment in extended re
  /x
    1234

/#rhubarb
  abcd/x
    abcd

/^abcd#rhubarb/x
    abcd

/^(a)\1{2,3}(.)/
    aaab
    aaaab
    aaaaab
    aaaaaab

/(?!^)abc/
    the abc
    abc

/(?=^)abc/
    abc
    the abc

/^[ab]{1,3}(ab*|b)/
    aabbbbb

/^[ab]{1,3}?(ab*|b)/
    aabbbbb

/^[ab]{1,3}?(ab*?|b)/
    aabbbbb

/^[ab]{1,3}(ab*?|b)/
    aabbbbb

/  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*                          # optional leading comment
(?:    (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
" (?:                      # opening quote...
[^\\\x80-\xff\n\015"]                #   Anything except backslash and quote
|                     #    or
\\ [^\x80-\xff]           #   Escaped something (something != CR)
)* "  # closing quote
)                    # initial word
(?:  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  \.  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*   (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
" (?:                      # opening quote...
[^\\\x80-\xff\n\015"]                #   Anything except backslash and quote
|                     #    or
\\ [^\x80-\xff]           #   Escaped something (something != CR)
)* "  # closing quote
)  )* # further okay, if led by a period
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  @  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*    (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|   \[                         # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*    #    stuff
\]                        #           ]
)                           # initial subdomain
(?:                                  #
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  \.                        # if led by a period...
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*   (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|   \[                         # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*    #    stuff
\]                        #           ]
)                     #   ...further okay
)*
# address
|                     #  or
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
" (?:                      # opening quote...
[^\\\x80-\xff\n\015"]                #   Anything except backslash and quote
|                     #    or
\\ [^\x80-\xff]           #   Escaped something (something != CR)
)* "  # closing quote
)             # one word, optionally followed by....
(?:
[^()<>@,;:".\\\[\]\x80-\xff\000-\010\012-\037]  |  # atom and space parts, or...
\(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)       |  # comments, or...

" (?:                      # opening quote...
[^\\\x80-\xff\n\015"]                #   Anything except backslash and quote
|                     #    or
\\ [^\x80-\xff]           #   Escaped something (something != CR)
)* "  # closing quote
# quoted strings
)*
<  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*                     # leading <
(?:  @  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*    (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|   \[                         # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*    #    stuff
\]                        #           ]
)                           # initial subdomain
(?:                                  #
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  \.                        # if led by a period...
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*   (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|   \[                         # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*    #    stuff
\]                        #           ]
)                     #   ...further okay
)*

(?:  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  ,  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  @  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*    (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|   \[                         # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*    #    stuff
\]                        #           ]
)                           # initial subdomain
(?:                                  #
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  \.                        # if led by a period...
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*   (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|   \[                         # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*    #    stuff
\]                        #           ]
)                     #   ...further okay
)*
)* # further okay, if led by comma
:                                # closing colon
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  )? #       optional route
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
" (?:                      # opening quote...
[^\\\x80-\xff\n\015"]                #   Anything except backslash and quote
|                     #    or
\\ [^\x80-\xff]           #   Escaped something (something != CR)
)* "  # closing quote
)                    # initial word
(?:  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  \.  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*   (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
" (?:                      # opening quote...
[^\\\x80-\xff\n\015"]                #   Anything except backslash and quote
|                     #    or
\\ [^\x80-\xff]           #   Escaped something (something != CR)
)* "  # closing quote
)  )* # further okay, if led by a period
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  @  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*    (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|   \[                         # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*    #    stuff
\]                        #           ]
)                           # initial subdomain
(?:                                  #
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  \.                        # if led by a period...
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*   (?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|   \[                         # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*    #    stuff
\]                        #           ]
)                     #   ...further okay
)*
#       address spec
(?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*  > #                  trailing >
# name and address
)  (?: [\040\t] |  \(
(?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  |  \( (?:  [^\\\x80-\xff\n\015()]  |  \\ [^\x80-\xff]  )* \)  )*
\)  )*                       # optional trailing comment
/x
    Alan Other <user\@dom.ain>
    <user\@dom.ain>
    user\@dom.ain
    \"A. Other\" <user.1234\@dom.ain> (a comment)
    A. Other <user.1234\@dom.ain> (a comment)
    \"/s=user/ou=host/o=place/prmd=uu.yy/admd= /c=gb/\"\@x400-re.lay
    A missing angle <user\@some.where
    The quick brown fox

/[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# optional leading comment
(?:
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
# Atom
|                       #  or
"                                     # "
[^\\\x80-\xff\n\015"] *                            #   normal
(?:  \\ [^\x80-\xff]  [^\\\x80-\xff\n\015"] * )*        #   ( special normal* )*
"                                     #        "
# Quoted string
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
\.
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
# Atom
|                       #  or
"                                     # "
[^\\\x80-\xff\n\015"] *                            #   normal
(?:  \\ [^\x80-\xff]  [^\\\x80-\xff\n\015"] * )*        #   ( special normal* )*
"                                     #        "
# Quoted string
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# additional words
)*
@
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
\[                            # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*     #    stuff
\]                           #           ]
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# optional trailing comments
(?:
\.
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
\[                            # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*     #    stuff
\]                           #           ]
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# optional trailing comments
)*
# address
|                             #  or
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
# Atom
|                       #  or
"                                     # "
[^\\\x80-\xff\n\015"] *                            #   normal
(?:  \\ [^\x80-\xff]  [^\\\x80-\xff\n\015"] * )*        #   ( special normal* )*
"                                     #        "
# Quoted string
)
# leading word
[^()<>@,;:".\\\[\]\x80-\xff\000-\010\012-\037] *               # "normal" atoms and or spaces
(?:
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
|
"                                     # "
[^\\\x80-\xff\n\015"] *                            #   normal
(?:  \\ [^\x80-\xff]  [^\\\x80-\xff\n\015"] * )*        #   ( special normal* )*
"                                     #        "
) # "special" comment or quoted string
[^()<>@,;:".\\\[\]\x80-\xff\000-\010\012-\037] *            #  more "normal"
)*
<
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# <
(?:
@
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
\[                            # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*     #    stuff
\]                           #           ]
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# optional trailing comments
(?:
\.
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
\[                            # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*     #    stuff
\]                           #           ]
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# optional trailing comments
)*
(?: ,
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
@
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
\[                            # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*     #    stuff
\]                           #           ]
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# optional trailing comments
(?:
\.
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
\[                            # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*     #    stuff
\]                           #           ]
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# optional trailing comments
)*
)*  # additional domains
:
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# optional trailing comments
)?     #       optional route
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
# Atom
|                       #  or
"                                     # "
[^\\\x80-\xff\n\015"] *                            #   normal
(?:  \\ [^\x80-\xff]  [^\\\x80-\xff\n\015"] * )*        #   ( special normal* )*
"                                     #        "
# Quoted string
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
\.
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
# Atom
|                       #  or
"                                     # "
[^\\\x80-\xff\n\015"] *                            #   normal
(?:  \\ [^\x80-\xff]  [^\\\x80-\xff\n\015"] * )*        #   ( special normal* )*
"                                     #        "
# Quoted string
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# additional words
)*
@
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
\[                            # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*     #    stuff
\]                           #           ]
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# optional trailing comments
(?:
\.
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
(?:
[^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]+    # some number of atom characters...
(?![^(\040)<>@,;:".\\\[\]\000-\037\x80-\xff]) # ..not followed by something that could be part of an atom
|
\[                            # [
(?: [^\\\x80-\xff\n\015\[\]] |  \\ [^\x80-\xff]  )*     #    stuff
\]                           #           ]
)
[\040\t]*                    # Nab whitespace.
(?:
\(                              #  (
[^\\\x80-\xff\n\015()] *                             #     normal*
(?:                                 #       (
(?:  \\ [^\x80-\xff]  |
\(                            #  (
[^\\\x80-\xff\n\015()] *                            #     normal*
(?:  \\ [^\x80-\xff]   [^\\\x80-\xff\n\015()] * )*        #     (special normal*)*
\)                           #                       )
)    #         special
[^\\\x80-\xff\n\015()] *                         #         normal*
)*                                  #            )*
\)                             #                )
[\040\t]* )*    # If comment found, allow more spaces.
# optional trailing comments
)*
#       address spec
>                    #                 >
# name and address
)
/x
    Alan Other <user\@dom.ain>
    <user\@dom.ain>
    user\@dom.ain
    \"A. Other\" <user.1234\@dom.ain> (a comment)
    A. Other <user.1234\@dom.ain> (a comment)
    \"/s=user/ou=host/o=place/prmd=uu.yy/admd= /c=gb/\"\@x400-re.lay
    A missing angle <user\@some.where
    The quick brown fox

/abc\0def\00pqr\000xyz\0000AB/
    abc\0def\00pqr\000xyz\0000AB
    abc456 abc\0def\00pqr\000xyz\0000ABCDE

/abc\x0def\x00pqr\x000xyz\x0000AB/
    abc\x0def\x00pqr\x000xyz\x0000AB
    abc456 abc\x0def\x00pqr\x000xyz\x0000ABCDE

/^[\000-\037]/
    \0A
    \01B
    \037C

/\0*/
    \0\0\0\0

/A\x0{2,3}Z/
    The A\x0\x0Z
    An A\0\x0\0Z
    A\0Z
    A\0\x0\0\x0Z

/^(cow|)\1(bell)/
    cowcowbell
    bell
    cowbell

/^\s/
    \040abc
    \x0cabc
    \nabc
    \rabc
    \tabc
    abc

/^a	b
  
    c/x
    abc

/^(a|)\1*b/
    ab
    aaaab
    b
    acb

/^(a|)\1+b/
    aab
    aaaab
    b
    ab

/^(a|)\1?b/
    ab
    aab
    b
    acb

/^(a|)\1{2}b/
    aaab
    b
    ab
    aab
    aaaab

/^(a|)\1{2,3}b/
    aaab
    aaaab
    b
    ab
    aab
    aaaaab

/ab{1,3}bc/
    abbbbc
    abbbc
    abbc
    abc
    abbbbbc

/([^.]*)\.([^:]*):[T ]+(.*)/
    track1.title:TBlah blah blah

/([^.]*)\.([^:]*):[T ]+(.*)/i
    track1.title:TBlah blah blah

/([^.]*)\.([^:]*):[t ]+(.*)/i
    track1.title:TBlah blah blah

/^[W-c]+$/
    WXY_^abc
    wxy

/^[W-c]+$/i
    WXY_^abc
    wxy_^ABC

/^[\x3f-\x5F]+$/i
    WXY_^abc
    wxy_^ABC

/^abc$/m
    abc
    qqq\nabc
    abc\nzzz
    qqq\nabc\nzzz

/^abc$/
    abc
    qqq\nabc
    abc\nzzz
    qqq\nabc\nzzz

/\Aabc\Z/m
    abc
    abc\n 
    qqq\nabc
    abc\nzzz
    qqq\nabc\nzzz
    
/\A(.)*\Z/s
    abc\ndef

/\A(.)*\Z/m
    abc\ndef

/(?:b)|(?::+)/
    b::c
    c::b

/[-az]+/
    az-
    b

/[az-]+/
    za-
    b

/[a\-z]+/
    a-z
    b

/[a-z]+/
    abcdxyz

/[\d-]+/
    12-34
    aaa

/[\d-z]+/
    12-34z
    aaa

/\x5c/
    \\

/\x20Z/
    the Zoo
    Zulu

/(abc)\1/i
    abcabc
    ABCabc
    abcABC

/ab{3cd/
    ab{3cd

/ab{3,cd/
    ab{3,cd

/ab{3,4a}cd/
    ab{3,4a}cd

/{4,5a}bc/
    {4,5a}bc

/^a.b/
    a\rb
    a\nb

/abc$/
    abc
    abc\n
    abc\ndef

/(abc)\123/
    abc\x53

/(abc)\223/
    abc\x93

/(abc)\323/
    abc\xd3

/(abc)\500/
    abc\x40
    abc\100

/(abc)\5000/
    abc\x400
    abc\x40\x30
    abc\1000
    abc\100\x30
    abc\100\060
    abc\100\60

/abc\81/
    abc\081
    abc\0\x38\x31

/abc\91/
    abc\091
    abc\0\x39\x31

/(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)(l)\12\123/
    abcdefghijkllS

/(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)\12\123/
    abcdefghijk\12S

/ab\gdef/
    abgdef

/a{0}bc/
    bc

/(a|(bc)){0,0}?xyz/
    xyz

/abc[\10]de/
    abc\010de

/abc[\1]de/
    abc\1de

/(abc)[\1]de/
    abc\1de

/a.b(?s)/
    a\nb

/^([^a])([^\b])([^c]*)([^d]{3,4})/
    baNOTccccd
    baNOTcccd
    baNOTccd
    bacccd
    anything
    b\bc   
    baccd

/[^a]/
    Abc
  
/[^a]/i
    Abc 

/[^a]+/
    AAAaAbc
  
/[^a]+/i
    AAAaAbc 

/[^a]+/
    bbb\nccc
   
/[^k]$/
    abc
    abk   
   
/[^k]{2,3}$/
    abc
    kbc
    kabc 
    abk
    akb
    akk 

/^\d{8,}\@.+[^k]$/
    12345678\@a.b.c.d
    123456789\@x.y.z
    12345678\@x.y.uk
    1234567\@a.b.c.d       

/(a)\1{8,}/
    aaaaaaaaa
    aaaaaaaaaa
    aaaaaaa   

/[^a]/
    aaaabcd
    aaAabcd 

/[^a]/i
    aaaabcd
    aaAabcd 

/[^az]/
    aaaabcd
    aaAabcd 

/[^az]/i
    aaaabcd
    aaAabcd 

/\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037\040\041\042\043\044\045\046\047\050\051\052\053\054\055\056\057\060\061\062\063\064\065\066\067\070\071\072\073\074\075\076\077\100\101\102\103\104\105\106\107\110\111\112\113\114\115\116\117\120\121\122\123\124\125\126\127\130\131\132\133\134\135\136\137\140\141\142\143\144\145\146\147\150\151\152\153\154\155\156\157\160\161\162\163\164\165\166\167\170\171\172\173\174\175\176\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377/
 \000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037\040\041\042\043\044\045\046\047\050\051\052\053\054\055\056\057\060\061\062\063\064\065\066\067\070\071\072\073\074\075\076\077\100\101\102\103\104\105\106\107\110\111\112\113\114\115\116\117\120\121\122\123\124\125\126\127\130\131\132\133\134\135\136\137\140\141\142\143\144\145\146\147\150\151\152\153\154\155\156\157\160\161\162\163\164\165\166\167\170\171\172\173\174\175\176\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377

/P[^*]TAIRE[^*]{1,6}?LL/
    xxxxxxxxxxxPSTAIREISLLxxxxxxxxx

/P[^*]TAIRE[^*]{1,}?LL/
    xxxxxxxxxxxPSTAIREISLLxxxxxxxxx

/(\.\d\d[1-9]?)\d+/
    1.230003938
    1.875000282   
    1.235  
                  
/(\.\d\d((?=0)|\d(?=\d)))/
    1.230003938      
    1.875000282
    1.235 
    
/a(?)b/
    ab 
 
/\b(foo)\s+(\w+)/i
    Food is on the foo table
    
/foo(.*)bar/
    The food is under the bar in the barn.
    
/foo(.*?)bar/  
    The food is under the bar in the barn.

/(.*)(\d*)/
    I have 2 numbers: 53147
    
/(.*)(\d+)/
    I have 2 numbers: 53147
 
/(.*?)(\d*)/
    I have 2 numbers: 53147

/(.*?)(\d+)/
    I have 2 numbers: 53147

/(.*)(\d+)$/
    I have 2 numbers: 53147

/(.*?)(\d+)$/
    I have 2 numbers: 53147

/(.*)\b(\d+)$/
    I have 2 numbers: 53147

/(.*\D)(\d+)$/
    I have 2 numbers: 53147

/^\D*(?!123)/
    ABC123
     
/^(\D*)(?=\d)(?!123)/
    ABC445
    ABC123
    
/^[W-]46]/
    W46]789 
    -46]789
    Wall
    Zebra
    42
    [abcd] 
    ]abcd[
       
/^[W-\]46]/
    W46]789 
    Wall
    Zebra
    Xylophone  
    42
    [abcd] 
    ]abcd[
    \\backslash 
    -46]789
    well
    
/\d\d\/\d\d\/\d\d\d\d/
    01/01/2000

/word (?:[a-zA-Z0-9]+ ){0,10}otherword/
  word cat dog elephant mussel cow horse canary baboon snake shark otherword
  word cat dog elephant mussel cow horse canary baboon snake shark

/word (?:[a-zA-Z0-9]+ ){0,300}otherword/
  word cat dog elephant mussel cow horse canary baboon snake shark the quick brown fox and the lazy dog and several other words getting close to thirty by now I hope

/^(a){0,0}/
    bcd
    abc
    aab     

/^(a){0,1}/
    bcd
    abc
    aab  

/^(a){0,2}/
    bcd
    abc
    aab  

/^(a){0,3}/
    bcd
    abc
    aab
    aaa   

/^(a){0,}/
    bcd
    abc
    aab
    aaa
    aaaaaaaa    

/^(a){1,1}/
    bcd
    abc
    aab  

/^(a){1,2}/
    bcd
    abc
    aab  

/^(a){1,3}/
    bcd
    abc
    aab
    aaa   

/^(a){1,}/
    bcd
    abc
    aab
    aaa
    aaaaaaaa    

/.*\.gif/
    borfle\nbib.gif\nno

/.{0,}\.gif/
    borfle\nbib.gif\nno

/.*\.gif/m
    borfle\nbib.gif\nno

/.*\.gif/s
    borfle\nbib.gif\nno

/.*\.gif/ms
    borfle\nbib.gif\nno
    
/.*$/
    borfle\nbib.gif\nno

/.*$/m
    borfle\nbib.gif\nno

/.*$/s
    borfle\nbib.gif\nno

/.*$/ms
    borfle\nbib.gif\nno
    
/.*$/
    borfle\nbib.gif\nno\n

/.*$/m
    borfle\nbib.gif\nno\n

/.*$/s
    borfle\nbib.gif\nno\n

/.*$/ms
    borfle\nbib.gif\nno\n
    
/(.*X|^B)/
    abcde\n1234Xyz
    BarFoo 
    abcde\nBar  

/(.*X|^B)/m
    abcde\n1234Xyz
    BarFoo 
    abcde\nBar  

/(.*X|^B)/s
    abcde\n1234Xyz
    BarFoo 
    abcde\nBar  

/(.*X|^B)/ms
    abcde\n1234Xyz
    BarFoo 
    abcde\nBar  

/(?s)(.*X|^B)/
    abcde\n1234Xyz
    BarFoo 
    abcde\nBar  

/(?s:.*X|^B)/
    abcde\n1234Xyz
    BarFoo 
    abcde\nBar  

/^.*B/
    abc\nB
     
/(?s)^.*B/
    abc\nB

/(?m)^.*B/
    abc\nB
     
/(?ms)^.*B/
    abc\nB

/(?ms)^B/
    abc\nB

/(?s)B$/
    B\n

/^[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]/
    123456654321
  
/^\d\d\d\d\d\d\d\d\d\d\d\d/
    123456654321 

/^[\d][\d][\d][\d][\d][\d][\d][\d][\d][\d][\d][\d]/
    123456654321
  
/^[abc]{12}/
    abcabcabcabc
    
/^[a-c]{12}/
    abcabcabcabc
    
/^(a|b|c){12}/
    abcabcabcabc 

/^[abcdefghijklmnopqrstuvwxy0123456789]/
    n
    z 

/abcde{0,0}/
    abcd
    abce  

/ab[cd]{0,0}e/
    abe
    abcde 
    
/ab(c){0,0}d/
    abd
    abcd   

/a(b*)/
    a
    ab
    abbbb
    bbbbb    
    
/ab\d{0}e/
    abe
    ab1e   
    
/"([^\\"]+|\\.)*"/
    the \"quick\" brown fox
    \"the \\\"quick\\\" brown fox\" 

/.*?/g+
    abc
  
/\b/g+
    abc 

/\b/+g
    abc 

//g
    abc

/<tr([\w\W\s\d][^<>]{0,})><TD([\w\W\s\d][^<>]{0,})>([\d]{0,}\.)(.*)((<BR>([\w\W\s\d][^<>]{0,})|[\s]{0,}))<\/a><\/TD><TD([\w\W\s\d][^<>]{0,})>([\w\W\s\d][^<>]{0,})<\/TD><TD([\w\W\s\d][^<>]{0,})>([\w\W\s\d][^<>]{0,})<\/TD><\/TR>/is
  <TR BGCOLOR='#DBE9E9'><TD align=left valign=top>43.<a href='joblist.cfm?JobID=94 6735&Keyword='>Word Processor<BR>(N-1286)</a></TD><TD align=left valign=top>Lega lstaff.com</TD><TD align=left valign=top>CA - Statewide</TD></TR>

/a[^a]b/
    acb
    a\nb
    
/a.b/
    acb
    a\nb   
    
/a[^a]b/s
    acb
    a\nb  
    
/a.b/s
    acb
    a\nb  

/^(b+?|a){1,2}?c/
    bac
    bbac
    bbbac
    bbbbac
    bbbbbac 

/^(b+|a){1,2}?c/
    bac
    bbac
    bbbac
    bbbbac
    bbbbbac 
    
/(?!\A)x/m
    x\nb\n
    a\bx\n  
    
/\x0{ab}/
    \0{ab} 

/(A|B)*?CD/
    CD 
    
/(A|B)*CD/
    CD 

/(AB)*?\1/
    ABABAB

/(AB)*\1/
    ABABAB
    
/(/
    doesn't matter

/(x)\2/
    doesn't matter

/((a{0,5}){0,5}){0,5}[c]/
    aaaaaaaaaac
    aaaaaaaaaa

/((a{0,5}){0,5})*[c]/
    aaaaaaaaaac
    aaaaaaaaaa

/(\b)*a/
    a

/(a)*b/
    ab

/(a|)*b/
    ab
    b
    x

/^(?:(a)|(b))*\1\2$/
    abab

/abc[^x]def/
    abcxabcydef

/^(a|\1x)*$/
    aax
    aaxa

//
    @{['']}

/^(?:(a)|(b))*$/
    ab

/[\0]/
    a
    \0

/[\1]/
    a
    \1

/\10()()()()()()()()()/
    doesn't matter

/\10()()()()()()()()()()/
    a

/a(?<)b/
    ab

/[]/
    doesn't matter

/[\]/
    doesn't matter

/()/
    a

/[\x]/
    x
    \0

/((a)*)*/
    a

/()a\1/
    a

/a\1()/
    a

/a(?i)a(?-i)a/
    aaa
    aAa
    aAA

/a(?i)a(?-i)a(?i)a(?-i)a/
    aaaaa
    aAaAa
    AaAaA
    aAAAa
    AaaaA
    AAAAA
    aaAAA
    AAaaa

/\x/
    a
    X
    \0

/[a-c-e]/
    a
    b
    d
    -

/[b-\d]/
    b
    c
    d
    -
    1

/[\d-f]/
    d
    e
    f
    -
    1

/[/
    doesn't matter

/]/
    ]
    a

/[]/
    doesn't matter

/[-a-c]/
    -
    a
    b
    d

/[a-c-]/
    -
    a
    b
    d

/[-]/
    a
    -

/[--]/
    a
    -

/[---]/
    a
    -

/[--b]/
    -
    a
    c

/[b--]/
    doesn't matter

/a{/
    a{

/a{}/
    a{}

/a{3/
    a{3

/a{3,/
    a{3,

/a{3, 3}/
    a{3,3}
    a{3, 3}
    aaa

/a{3, 3}/x
    a{3,3}
    a{3, 3}
    aaa

/a{3, }/
    a{3,}
    a{3, }
    aaa

/a{3, }/x
    a{3,}
    a{3, }
    aaa

/\x x/
    \0 x
    \0x

/\x x/x
    \0 x
    \0x

/\x 3/
    \0003
    \000 3
    x3
    x 3

/\x 3/x
    \0003
    \000 3
    x3
    x 3

/^a{ 1}$/
    a
    a{ 1}
    a{1}

/^a{ 1}$/x
    a
    a{ 1}
    a{1}

/{}/
    {}
    a

/{1}/
    doesn't matter

/*/
    doesn't matter

/|/
    x

/\0000/
    \0000

/a(?<)b/
    ab

/a(?i)b/
    ab
    aB
    Ab

/a(?i=a)/
    doesn't matter

/a(?<=a){3000}a/
  aa
  xa
  ax

/a(?!=a){3000}a/
  aa
  ax
  xa

/a(){3000}a/
  aa
  ax
  xa

/a(?:){3000}a/
  aa
  ax

/a(?<=a)*a/
  aa
  ax
  xa

/a(?!=a)*a/
  aa
  ax
  xa

/a()*a/
  aa
  ax
  xa

/a(?:)*a/
  aa
  ax
  xa

/x(?<=a)*a/
  aa
  xa
  ax

/a(?<=(a))*\1/
  aa

/a(?<=(a))*?\1/
  aa

/(?=(a)\1)*aa/
  aa

/^((a|b){2,5}){2}$/
  aaaaabbbbb

/^(b*|ba){1,2}bc/
  babc
  bbabc
  bababc
  bababbc
  babababc

/^a{4,5}(?:c|a)c$/
  aaaaac
  aaaaaac

/^(a|){4,5}(?:c|a)c$/
  aaaaac
  aaaaaac

/(?m:^).abc$/
  eeexabc
  eee\nxabc

/(?m:^)abc/
  abc
  \nabc


/^abc/
  abc
  \nabc

/\Aabc/
  abc
  \nabc

/(?<!bar)foo/
    foo
    catfood
    arfootle
    rfoosh
    barfoo
    towbarfoo

/\w{3}(?<!bar)foo/
    catfood
    foo
    barfoo
    towbarfoo

/(?<=(foo)a)bar/
    fooabar
    bar
    foobbar
      
/\Aabc\z/m
    abc
    abc\n   
    qqq\nabc
    abc\nzzz
    qqq\nabc\nzzz

"(?>.*/)foo"
    /this/is/a/very/long/line/in/deed/with/very/many/slashes/in/it/you/see/

"(?>.*/)foo"
    /this/is/a/very/long/line/in/deed/with/very/many/slashes/in/and/foo

/(?>(\.\d\d[1-9]?))\d+/
    1.230003938
    1.875000282
    1.235 

/^((?>\w+)|(?>\s+))*$/
    now is the time for all good men to come to the aid of the party
    this is not a line with only words and spaces!
    
/(\d+)(\w)/
    12345a
    12345+ 

/((?>\d+))(\w)/
    12345a
    12345+ 

/(?>a+)b/
    aaab

/((?>a+)b)/
    aaab

/(?>(a+))b/
    aaab

/(?>b)+/
    aaabbbccc

/(?>a+|b+|c+)*c/
    aaabbbbccccd

/((?>[^()]+)|\([^()]*\))+/
    ((abc(ade)ufh()()x
    
/\(((?>[^()]+)|\([^()]+\))+\)/ 
    (abc)
    (abc(def)xyz)
    ((()aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa   

/a(?-i)b/i
    ab
    Ab
    aB
    AB
        
/(a (?x)b c)d e/
    a bcd e
    a b cd e
    abcd e   
    a bcde 
 
/(a b(?x)c d (?-x)e f)/
    a bcde f
    abcdef  

/(a(?i)b)c/
    abc
    aBc
    abC
    aBC  
    Abc
    ABc
    ABC
    AbC
    
/a(?i:b)c/
    abc
    aBc
    ABC
    abC
    aBC
    
/a(?i:b)*c/
    aBc
    aBBc
    aBC
    aBBC
    
/a(?=b(?i)c)\w\wd/
    abcd
    abCd
    aBCd
    abcD     
    
/(?s-i:more.*than).*million/i
    more than million
    more than MILLION
    more \n than Million 
    MORE THAN MILLION    
    more \n than \n million 

/(?:(?s-i)more.*than).*million/i
    more than million
    more than MILLION
    more \n than Million 
    MORE THAN MILLION    
    more \n than \n million 
    
/(?>a(?i)b+)+c/ 
    abc
    aBbc
    aBBc 
    Abc
    abAb    
    abbC 
    
/(?=a(?i)b)\w\wc/
    abc
    aBc
    Ab 
    abC
    aBC     
    
/(?<=a(?i)b)(\w\w)c/
    abxxc
    aBxxc
    Abxxc
    ABxxc
    abxxC      

/(?:(a)|b)(?(1)A|B)/
    aA
    bB
    aB
    bA    

/^(a)?(?(1)a|b)+$/
    aa
    b
    bb  
    ab   

/^(?(?=abc)\w{3}:|\d\d)$/
    abc:
    12
    123
    xyz    

/^(?(?!abc)\d\d|\w{3}:)$/
    abc:
    12
    123
    xyz    
    
/(?(?<=foo)bar|cat)/
    foobar
    cat
    fcat
    focat   
    foocat  

/(?(?<!foo)cat|bar)/
    foobar
    cat
    fcat
    focat   
    foocat  

/( \( )? [^()]+ (?(1) \) |) /x
    abcd
    (abcd)
    the quick (abcd) fox
    (abcd   

/( \( )? [^()]+ (?(1) \) ) /x
    abcd
    (abcd)
    the quick (abcd) fox
    (abcd   

/^(?(2)a|(1)(2))+$/
    12
    12a
    12aa
    1234    

/((?i)blah)\s+\1/
    blah blah
    BLAH BLAH
    Blah Blah
    blaH blaH
    blah BLAH
    Blah blah      
    blaH blah 

/((?i)blah)\s+(?i:\1)/
    blah blah
    BLAH BLAH
    Blah Blah
    blaH blaH
    blah BLAH
    Blah blah      
    blaH blah 

/(?>a*)*/
    a
    aa
    aaaa
    
/(abc|)+/
    abc
    abcabc
    abcabcabc
    xyz      

/([a]*)*/
    a
    aaaaa 
 
/([ab]*)*/
    a
    b
    ababab
    aaaabcde
    bbbb    
 
/([^a]*)*/
    b
    bbbb
    aaa   
 
/([^ab]*)*/
    cccc
    abab  
 
/([a]*?)*/
    a
    aaaa 
 
/([ab]*?)*/
    a
    b
    abab
    baba   
 
/([^a]*?)*/
    b
    bbbb
    aaa   
 
/([^ab]*?)*/
    c
    cccc
    baba   
 
/(?>a*)*/
    a
    aaabcde 
 
/((?>a*))*/
    aaaaa
    aabbaa 
 
/((?>a*?))*/
    aaaaa
    aabbaa 

/(?(?=[^a-z]+[a-z])  \d{2}-[a-z]{3}-\d{2}  |  \d{2}-\d{2}-\d{2} ) /x
    12-sep-98
    12-09-98
    sep-12-98
        
/(?<=(foo))bar\1/
    foobarfoo
    foobarfootling 
    foobar
    barfoo   

/(?i:saturday|sunday)/
    saturday
    sunday
    Saturday
    Sunday
    SATURDAY
    SUNDAY
    SunDay
    
/(a(?i)bc|BB)x/
    abcx
    aBCx
    bbx
    BBx
    abcX
    aBCX
    bbX
    BBX               

/^([ab](?i)[cd]|[ef])/
    ac
    aC
    bD
    elephant
    Europe 
    frog
    France
    Africa     

/^(ab|a(?i)[b-c](?m-i)d|x(?i)y|z)/
    ab
    aBd
    xy
    xY
    zebra
    Zambesi
    aCD  
    XY  

/(?<=foo\n)^bar/m
    foo\nbar
    bar
    baz\nbar   

/(?<=(?<!foo)bar)baz/
    barbaz
    barbarbaz 
    koobarbaz 
    baz
    foobarbaz 

/^(a\1?){4}$/
    a
    aa
    aaa
    aaaa
    aaaaa
    aaaaaa
    aaaaaaa
    aaaaaaaa
    aaaaaaaaa
    aaaaaaaaaa
    aaaaaaaaaaa
    aaaaaaaaaaaa
    aaaaaaaaaaaaa
    aaaaaaaaaaaaaa
    aaaaaaaaaaaaaaa
    aaaaaaaaaaaaaaaa               

/^(a\1?)(a\1?)(a\2?)(a\3?)$/
    a
    aa
    aaa
    aaaa
    aaaaa
    aaaaaa
    aaaaaaa
    aaaaaaaa
    aaaaaaaaa
    aaaaaaaaaa
    aaaaaaaaaaa
    aaaaaaaaaaaa
    aaaaaaaaaaaaa
    aaaaaaaaaaaaaa
    aaaaaaaaaaaaaaa
    aaaaaaaaaaaaaaaa               

/abc/
    abc
    xabcy
    ababc
    xbc
    axc
    abx

/ab*c/
    abc

/ab*bc/
    abc
    abbc
    abbbbc

/.{1}/
    abbbbc

/.{3,4}/
    abbbbc

/ab{0,}bc/
    abbbbc

/ab+bc/
    abbc
    abc
    abq

/ab{1,}bc/

/ab+bc/
    abbbbc

/ab{1,}bc/
    abbbbc

/ab{1,3}bc/
    abbbbc

/ab{3,4}bc/
    abbbbc

/ab{4,5}bc/
    abq
    abbbbc

/ab?bc/
    abbc
    abc

/ab{0,1}bc/
    abc

/ab?bc/

/ab?c/
    abc

/ab{0,1}c/
    abc

/^abc$/
    abc
    abbbbc
    abcc

/^abc/
    abcc

/^abc$/

/abc$/
    aabc
    aabc
    aabcd

/^/
    abc

/$/
    abc

/a.c/
    abc
    axc

/a.*c/
    axyzc

/a[bc]d/
    abd
    axyzd
    abc

/a[b-d]e/
    ace

/a[b-d]/
    aac

/a[-b]/
    a-

/a[b-]/
    a-

/a]/
    a]

/a[]]b/
    a]b

/a[^bc]d/
    aed
    abd
    abd

/a[^-b]c/
    adc

/a[^]b]c/
    adc
    a-c
    a]c

/\ba\b/
    a-
    -a
    -a-

/\by\b/
    xy
    yz
    xyz

/\Ba\B/
    a-
    -a
    -a-

/\By\b/
    xy

/\by\B/
    yz

/\By\B/
    xyz

/\w/
    a

/\W/
    -
    -
    a

/a\sb/
    a b

/a\Sb/
    a-b
    a-b
    a b

/\d/
    1

/\D/
    -
    -
    1

/[\w]/
    a

/[\W]/
    -
    -
    a

/a[\s]b/
    a b

/a[\S]b/
    a-b
    a-b
    a b

/[\d]/
    1

/[\D]/
    -
    -
    1

/ab|cd/
    abc
    abcd

/()ef/
    def

/$b/

/a\(b/
    a(b

/a\(*b/
    ab
    a((b

/a\\b/
    a\b

/((a))/
    abc

/(a)b(c)/
    abc

/a+b+c/
    aabbabc

/a{1,}b{1,}c/
    aabbabc

/a.+?c/
    abcabc

/(a+|b)*/
    ab

/(a+|b){0,}/
    ab

/(a+|b)+/
    ab

/(a+|b){1,}/
    ab

/(a+|b)?/
    ab

/(a+|b){0,1}/
    ab

/[^ab]*/
    cde

/abc/
    b
    

/a*/
    

/([abc])*d/
    abbbcd

/([abc])*bcd/
    abcd

/a|b|c|d|e/
    e

/(a|b|c|d|e)f/
    ef

/abcd*efg/
    abcdefg

/ab*/
    xabyabbbz
    xayabbbz

/(ab|cd)e/
    abcde

/[abhgefdc]ij/
    hij

/^(ab|cd)e/

/(abc|)ef/
    abcdef

/(a|b)c*d/
    abcd

/(ab|ab*)bc/
    abc

/a([bc]*)c*/
    abc

/a([bc]*)(c*d)/
    abcd

/a([bc]+)(c*d)/
    abcd

/a([bc]*)(c+d)/
    abcd

/a[bcd]*dcdcde/
    adcdcde

/a[bcd]+dcdcde/
    abcde
    adcdcde

/(ab|a)b*c/
    abc

/((a)(b)c)(d)/
    abcd

/[a-zA-Z_][a-zA-Z0-9_]*/
    alpha

/^a(bc+|b[eh])g|.h$/
    abh

/(bc+d$|ef*g.|h?i(j|k))/
    effgz
    ij
    reffgz
    effg
    bcdd

/((((((((((a))))))))))/
    a

/((((((((((a))))))))))\10/
    aa

/(((((((((a)))))))))/
    a

/multiple words of text/
    aa
    uh-uh

/multiple words/
    multiple words, yeah

/(.*)c(.*)/
    abcde

/\((.*), (.*)\)/
    (a, b)

/[k]/

/abcd/
    abcd

/a(bc)d/
    abcd

/a[-]?c/
    ac

/(abc)\1/
    abcabc

/([a-c]*)\1/
    abcabc

/(a)|\1/
    a
    ab
    x

/(([a-c])b*?\2)*/
    ababbbcbc

/(([a-c])b*?\2){3}/
    ababbbcbc

/((\3|b)\2(a)x)+/
    aaaxabaxbaaxbbax

/((\3|b)\2(a)){2,}/
    bbaababbabaaaaabbaaaabba

/abc/i
    ABC
    XABCY
    ABABC
    aaxabxbaxbbx
    XBC
    AXC
    ABX

/ab*c/i
    ABC

/ab*bc/i
    ABC
    ABBC

/ab*?bc/i
    ABBBBC

/ab{0,}?bc/i
    ABBBBC

/ab+?bc/i
    ABBC

/ab+bc/i
    ABC
    ABQ

/ab{1,}bc/i

/ab+bc/i
    ABBBBC

/ab{1,}?bc/i
    ABBBBC

/ab{1,3}?bc/i
    ABBBBC

/ab{3,4}?bc/i
    ABBBBC

/ab{4,5}?bc/i
    ABQ
    ABBBBC

/ab??bc/i
    ABBC
    ABC

/ab{0,1}?bc/i
    ABC

/ab??bc/i

/ab??c/i
    ABC

/ab{0,1}?c/i
    ABC

/^abc$/i
    ABC
    ABBBBC
    ABCC

/^abc/i
    ABCC

/^abc$/i

/abc$/i
    AABC

/^/i
    ABC

/$/i
    ABC

/a.c/i
    ABC
    AXC

/a.*?c/i
    AXYZC

/a.*c/i
    AABC
    AXYZD

/a[bc]d/i
    ABD

/a[b-d]e/i
    ACE
    ABC
    ABD

/a[b-d]/i
    AAC

/a[-b]/i
    A-

/a[b-]/i
    A-

/a]/i
    A]

/a[]]b/i
    A]B

/a[^bc]d/i
    AED

/a[^-b]c/i
    ADC
    ABD
    A-C

/a[^]b]c/i
    ADC

/ab|cd/i
    ABC
    ABCD

/()ef/i
    DEF

/$b/i
    A]C
    B

/a\(b/i
    A(B

/a\(*b/i
    AB
    A((B

/a\\b/i
    A\B

/((a))/i
    ABC

/(a)b(c)/i
    ABC

/a+b+c/i
    AABBABC

/a{1,}b{1,}c/i
    AABBABC

/a.+?c/i
    ABCABC

/a.*?c/i
    ABCABC

/a.{0,5}?c/i
    ABCABC

/(a+|b)*/i
    AB

/(a+|b){0,}/i
    AB

/(a+|b)+/i
    AB

/(a+|b){1,}/i
    AB

/(a+|b)?/i
    AB

/(a+|b){0,1}/i
    AB

/(a+|b){0,1}?/i
    AB

/[^ab]*/i
    CDE

/abc/i

/a*/i
    

/([abc])*d/i
    ABBBCD

/([abc])*bcd/i
    ABCD

/a|b|c|d|e/i
    E

/(a|b|c|d|e)f/i
    EF

/abcd*efg/i
    ABCDEFG

/ab*/i
    XABYABBBZ
    XAYABBBZ

/(ab|cd)e/i
    ABCDE

/[abhgefdc]ij/i
    HIJ

/^(ab|cd)e/i
    ABCDE

/(abc|)ef/i
    ABCDEF

/(a|b)c*d/i
    ABCD

/(ab|ab*)bc/i
    ABC

/a([bc]*)c*/i
    ABC

/a([bc]*)(c*d)/i
    ABCD

/a([bc]+)(c*d)/i
    ABCD

/a([bc]*)(c+d)/i
    ABCD

/a[bcd]*dcdcde/i
    ADCDCDE

/a[bcd]+dcdcde/i

/(ab|a)b*c/i
    ABC

/((a)(b)c)(d)/i
    ABCD

/[a-zA-Z_][a-zA-Z0-9_]*/i
    ALPHA

/^a(bc+|b[eh])g|.h$/i
    ABH

/(bc+d$|ef*g.|h?i(j|k))/i
    EFFGZ
    IJ
    REFFGZ
    ADCDCDE
    EFFG
    BCDD

/((((((((((a))))))))))/i
    A

/((((((((((a))))))))))\10/i
    AA

/(((((((((a)))))))))/i
    A

/(?:(?:(?:(?:(?:(?:(?:(?:(?:(a))))))))))/i
    A

/(?:(?:(?:(?:(?:(?:(?:(?:(?:(a|b|c))))))))))/i
    C

/multiple words of text/i
    AA
    UH-UH

/multiple words/i
    MULTIPLE WORDS, YEAH

/(.*)c(.*)/i
    ABCDE

/\((.*), (.*)\)/i
    (A, B)

/[k]/i

/abcd/i
    ABCD

/a(bc)d/i
    ABCD

/a[-]?c/i
    AC

/(abc)\1/i
    ABCABC

/([a-c]*)\1/i
    ABCABC

/a(?!b)./
    abad

/a(?=d)./
    abad

/a(?=c|d)./
    abad

/a(?:b|c|d)(.)/
    ace

/a(?:b|c|d)*(.)/
    ace

/a(?:b|c|d)+?(.)/
    ace
    acdbcdbe

/a(?:b|c|d)+(.)/
    acdbcdbe

/a(?:b|c|d){2}(.)/
    acdbcdbe

/a(?:b|c|d){4,5}(.)/
    acdbcdbe

/a(?:b|c|d){4,5}?(.)/
    acdbcdbe

/((foo)|(bar))*/
    foobar

/a(?:b|c|d){6,7}(.)/
    acdbcdbe

/a(?:b|c|d){6,7}?(.)/
    acdbcdbe

/a(?:b|c|d){5,6}(.)/
    acdbcdbe

/a(?:b|c|d){5,6}?(.)/
    acdbcdbe

/a(?:b|c|d){5,7}(.)/
    acdbcdbe

/a(?:b|c|d){5,7}?(.)/
    acdbcdbe

/a(?:b|(c|e){1,2}?|d)+?(.)/
    ace

/^(.+)?B/
    AB

/^([^a-z])|(\^)$/
    .

/^[<>]&/
    <&OUT

/^(a\1?){4}$/
    aaaaaaaaaa
    AB
    aaaaaaaaa
    aaaaaaaaaaa

/^(a(?(1)\1)){4}$/
    aaaaaaaaaa
    aaaaaaaaa
    aaaaaaaaaaa

/(?:(f)(o)(o)|(b)(a)(r))*/
    foobar

/(?<=a)b/
    ab
    cb
    b

/(?<!c)b/
    ab
    b
    b

/(?:..)*a/
    aba

/(?:..)*?a/
    aba

/^(?:b|a(?=(.)))*\1/
    abc

/^(){3,5}/
    abc

/^(a+)*ax/
    aax

/^((a|b)+)*ax/
    aax

/^((a|bc)+)*ax/
    aax

/(a|x)*ab/
    cab

/(a)*ab/
    cab

/(?:(?i)a)b/
    ab

/((?i)a)b/
    ab

/(?:(?i)a)b/
    Ab

/((?i)a)b/
    Ab

/(?:(?i)a)b/
    cb
    aB

/((?i)a)b/

/(?i:a)b/
    ab

/((?i:a))b/
    ab

/(?i:a)b/
    Ab

/((?i:a))b/
    Ab

/(?i:a)b/
    aB
    aB

/((?i:a))b/

/(?:(?-i)a)b/i
    ab

/((?-i)a)b/i
    ab

/(?:(?-i)a)b/i
    aB

/((?-i)a)b/i
    aB

/(?:(?-i)a)b/i
    aB
    Ab

/((?-i)a)b/i

/(?:(?-i)a)b/i
    aB

/((?-i)a)b/i
    aB

/(?:(?-i)a)b/i
    Ab
    AB

/((?-i)a)b/i

/(?-i:a)b/i
    ab

/((?-i:a))b/i
    ab

/(?-i:a)b/i
    aB

/((?-i:a))b/i
    aB

/(?-i:a)b/i
    AB
    Ab

/((?-i:a))b/i

/(?-i:a)b/i
    aB

/((?-i:a))b/i
    aB

/(?-i:a)b/i
    Ab
    AB

/((?-i:a))b/i

/((?-i:a.))b/i
    AB
    a\nB

/((?s-i:a.))b/i
    a\nB

/(?:c|d)(?:)(?:a(?:)(?:b)(?:b(?:))(?:b(?:)(?:b)))/
    cabbbb

/(?:c|d)(?:)(?:aaaaaaaa(?:)(?:bbbbbbbb)(?:bbbbbbbb(?:))(?:bbbbbbbb(?:)(?:bbbbbbbb)))/
    caaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb

/(ab)\d\1/i
    Ab4ab
    ab4Ab

/foo\w*\d{4}baz/
    foobar1234baz

/x(~~)*(?:(?:F)?)?/
    x~~

/^a(?#xxx){3}c/
    aaac

/^a(?#xxx)(?#xxx){3}c/
    aaac

/^a (?#xxx) (?#yyy) {3}c/x
    aaac

/(?<![cd])b/
    B\nB
    dbcb

/(?<![cd])[ab]/
    dbaacb

/(?<!(c|d))b/

/(?<!(c|d))[ab]/
    dbaacb

/(?<!cd)[ab]/
    cdaccb

/^(?:a?b?)*$/
    dbcb
    a--

/((?s)^a(.))((?m)^b$)/
    a\nb\nc\n

/((?m)^b$)/
    a\nb\nc\n

/(?m)^b/
    a\nb\n

/(?m)^(b)/
    a\nb\n

/((?m)^b)/
    a\nb\n

/\n((?m)^b)/
    a\nb\n

/((?s).)c(?!.)/
    a\nb\nc\n
    a\nb\nc\n

/((?s)b.)c(?!.)/
    a\nb\nc\n
    a\nb\nc\n

/^b/

/()^b/
    a\nb\nc\n
    a\nb\nc\n

/((?m)^b)/
    a\nb\nc\n

/(?(1)a|b)/

/(?(1)b|a)/
    a

/(x)?(?(1)a|b)/
    a
    a

/(x)?(?(1)b|a)/
    a

/()?(?(1)b|a)/
    a

/()(?(1)b|a)/

/()?(?(1)a|b)/
    a

/^(\()?blah(?(1)(\)))$/
    (blah)
    blah
    a
    blah)
    (blah

/^(\(+)?blah(?(1)(\)))$/
    (blah)
    blah
    blah)
    (blah

/(?(?!a)a|b)/

/(?(?!a)b|a)/
    a

/(?(?=a)b|a)/
    a
    a

/(?(?=a)a|b)/
    a

/(?=(a+?))(\1ab)/
    aaab

/^(?=(a+?))\1ab/

/(\w+:)+/
    one:

/$(?<=^(a))/
    a

/(?=(a+?))(\1ab)/
    aaab

/^(?=(a+?))\1ab/
    aaab
    aaab

/([\w:]+::)?(\w+)$/
    abcd
    xy:z:::abcd

/^[^bcd]*(c+)/
    aexycd

/(a*)b+/
    caab

/([\w:]+::)?(\w+)$/
    abcd
    xy:z:::abcd
    abcd:
    abcd:

/^[^bcd]*(c+)/
    aexycd

/(>a+)ab/

/(?>a+)b/
    aaab

/([[:]+)/
    a:[b]:

/([[=]+)/
    a=[b]=

/([[.]+)/
    a.[b].

/((?>a+)b)/
    aaab

/(?>(a+))b/
    aaab

/((?>[^()]+)|\([^()]*\))+/
    ((abc(ade)ufh()()x

/a\Z/
    aaab
    a\nb\n

/b\Z/
    a\nb\n

/b\z/

/b\Z/
    a\nb

/b\z/
    a\nb
    
/^(?>(?(1)\.|())[^\W_](?>[a-z0-9-]*[^\W_])?)+$/
    a
    abc
    a-b
    0-9 
    a.b
    5.6.7  
    the.quick.brown.fox
    a100.b200.300c  
    12-ab.1245 
    \
    .a
    -a
    a-
    a.  
    a_b 
    a.-
    a..  
    ab..bc 
    the.quick.brown.fox-
    the.quick.brown.fox.
    the.quick.brown.fox_
    the.quick.brown.fox+       

/(?>.*)(?<=(abcd|wxyz))/
    alphabetabcd
    endingwxyz
    a rather long string that doesn't end with one of them

/word (?>(?:(?!otherword)[a-zA-Z0-9]+ ){0,30})otherword/
    word cat dog elephant mussel cow horse canary baboon snake shark otherword
    word cat dog elephant mussel cow horse canary baboon snake shark
  
/word (?>[a-zA-Z0-9]+ ){0,30}otherword/
    word cat dog elephant mussel cow horse canary baboon snake shark the quick brown fox and the lazy dog and several other words getting close to thirty by now I hope

/(?<=\d{3}(?!999))foo/
    999foo
    123999foo 
    123abcfoo
    
/(?<=(?!...999)\d{3})foo/
    999foo
    123999foo 
    123abcfoo

/(?<=\d{3}(?!999)...)foo/
    123abcfoo
    123456foo 
    123999foo  
    
/(?<=\d{3}...)(?<!999)foo/
    123abcfoo   
    123456foo 
    123999foo  

/<a[\s]+href[\s]*=[\s]*          # find <a href=
 ([\"\'])?                       # find single or double quote
 (?(1) (.*?)\1 | ([^\s]+))       # if quote found, match up to next matching
                                 # quote, otherwise match up to next space
/isx
    <a href=abcd xyz
    <a href=\"abcd xyz pqr\" cats
    <a href=\'abcd xyz pqr\' cats

/<a\s+href\s*=\s*                # find <a href=
 (["'])?                         # find single or double quote
 (?(1) (.*?)\1 | (\S+))          # if quote found, match up to next matching
                                 # quote, otherwise match up to next space
/isx
    <a href=abcd xyz
    <a href=\"abcd xyz pqr\" cats
    <a href       =       \'abcd xyz pqr\' cats

/<a\s+href(?>\s*)=(?>\s*)        # find <a href=
 (["'])?                         # find single or double quote
 (?(1) (.*?)\1 | (\S+))          # if quote found, match up to next matching
                                 # quote, otherwise match up to next space
/isx
    <a href=abcd xyz
    <a href=\"abcd xyz pqr\" cats
    <a href       =       \'abcd xyz pqr\' cats

/((Z)+|A)*/
    ZABCDEFG

/(Z()|A)*/
    ZABCDEFG

/(Z(())|A)*/
    ZABCDEFG

/((?>Z)+|A)*/
    ZABCDEFG

/((?>)+|A)*/
    ZABCDEFG

/a*/g
    abbab

/^[a-\d]/
    abcde
    -things
    0digit
    bcdef    

/^[\d-a]/
    abcde
    -things
    0digit
    bcdef

/(?<=abc).*(?=def)/
    abcdef
    abcxdef
    abcxdefxdef

/(?<=abc).*?(?=def)/
    abcdef
    abcxdef
    abcxdefxdef

/(?<=abc).+(?=def)/
    abcdef
    abcxdef
    abcxdefxdef

/(?<=abc).+?(?=def)/
    abcdef
    abcxdef
    abcxdefxdef

/(?<=\b)(.*)/
    -abcdef
    abcdef

/(?<=\B)(.*)/
    -abcdef
    abcdef

/^'[ab]'/
    'a'
    'b'
    x'a'
    'a'x
    'ab'

/^'[ab]'$/
    'a'
    'b'
    x'a'
    'a'x
    'ab'

/'[ab]'$/
    'a'
    'b'
    x'a'
    'a'x
    'ab'

/'[ab]'/
    'a'
    'b'
    x'a'
    'a'x
    'ab'

/abc\x1B/
    abc
    abcE

/abc[\x1Bx]/
    abcx
    abcE

/^\Qa*\x1B$/
    a*
    a

/\Qa*x\x1B/
    a*x
    a*

/\Qa*x/
    a*x
    a*

/\Q\Qa*x\x1B\x1B/
    a*x
    a\\*x

/\Q\Qa*x\x1B/
    a*x
    a\\*x

/a\Q[x\x1B]/
    a[x]
    ax

/a#comment\Q...
{2}/x
    a
    aa

/a(?#comment\Q...
){2}/x
    a
    aa

/(?x)a#\Q
./
    a.
    aa

/ab(?=.*q)cd/
    abcdxklqj

/a(?!.*$)b/
    ab

/.{2}[a-z]/
    Axi