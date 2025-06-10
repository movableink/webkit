kind: manual

build-depends:
- freedesktop-sdk.bst:components/rust.bst
- freedesktop-sdk.bst:components/pkg-config.bst

depends:
- freedesktop-sdk.bst:bootstrap-import.bst
- freedesktop-sdk.bst:components/gstreamer.bst
- freedesktop-sdk.bst:components/gstreamer-plugins-base.bst

environment:
  PATH: /usr/bin:/usr/lib/sdk/rust/bin

config:
  build-commands:
  - |
    cargo --offline build --release

  install-commands:
  - |
    install -D -m a+r -t "%{install-root}%{libdir}/gstreamer-1.0" ./target/release/libgst*.so

public:
  bst:
    integration-commands:
    - |
      PAGER=cat gst-inspect-1.0 fmp4

sources:
- kind: tar
  url: crates:crates/gst-plugin-fmp4/gst-plugin-fmp4-0.13.6.crate
  ref: d0b1f92f252a0ec17a1db62a6d0f3bd57bb36727d1d7b756b0a969571b7a7981
- kind: cargo
  ref:
  - name: aho-corasick
    version: 1.1.3
    sha: 8e60d3430d3a69478ad0993f19238d2df97c507009a52b3c10addcd7f6bcb916
  - name: android-tzdata
    version: 0.1.1
    sha: e999941b234f3131b00bc13c22d06e8c5ff726d1b6318ac7eb276997bbb4fef0
  - name: android_system_properties
    version: 0.1.5
    sha: 819e7219dbd41043ac279b19830f2efc897156490d7fd6ea916720117ee66311
  - name: anyhow
    version: 1.0.98
    sha: e16d2d3311acee920a9eb8d33b8cbc1787ce4a264e85f964c2404b969bdcd487
  - name: atomic_refcell
    version: 0.1.13
    sha: 41e67cd8309bbd06cd603a9e693a784ac2e5d1e955f11286e355089fcab3047c
  - name: autocfg
    version: 1.4.0
    sha: ace50bade8e6234aa140d9a2f552bbee1db4d353f69b8217bc503490fc1a9f26
  - name: base64
    version: 0.22.1
    sha: 72b3254f16251a8381aa12e40e3c4d2f0199f8c6508fbecb9d91f575e0fbb8c6
  - name: base64-serde
    version: 0.8.0
    sha: 77c6d128af408d8ebd08331f0331cf2cf20d19e6c44a7aec58791641ecc8c0b5
  - name: bitflags
    version: 2.9.0
    sha: 5c8214115b7bf84099f1309324e63141d4c5d7cc26862f97a0a857dbefe165bd
  - name: bitstream-io
    version: 2.3.0
    sha: 7c12d1856e42f0d817a835fe55853957c85c8c8a470114029143d3f12671446e
  - name: bumpalo
    version: 3.17.0
    sha: 1628fb46dfa0b37568d12e5edd512553eccf6a22a78e8bde00bb4aed84d5bdbf
  - name: byteorder
    version: 1.5.0
    sha: 1fd0f2584146f6f2ef48085050886acf353beff7305ebd1ae69500e27c67f64b
  - name: bytes
    version: 1.10.1
    sha: d71b6127be86fdcfddb610f7182ac57211d4b18a3e9c82eb2d17662f2227ad6a
  - name: cc
    version: 1.2.22
    sha: 32db95edf998450acc7881c932f94cd9b05c87b4b2599e8bab064753da4acfd1
  - name: cfg-expr
    version: 0.17.2
    sha: 8d4ba6e40bd1184518716a6e1a781bf9160e286d219ccdb8ab2612e74cfe4789
  - name: cfg-if
    version: 1.0.0
    sha: baf1de4339761588bc0619e3cbc0120ee582ebb74b53b4efbf79117bd2da40fd
  - name: chrono
    version: 0.4.41
    sha: c469d952047f47f91b68d1cba3f10d63c11d73e4636f24f08daf0278abf01c4d
  - name: core-foundation-sys
    version: 0.8.7
    sha: 773648b94d0e5d620f64f280777445740e61fe701025087ec8b57f45c791888b
  - name: darling
    version: 0.20.11
    sha: fc7f46116c46ff9ab3eb1597a45688b6715c6e628b5c133e288e709a29bcb4ee
  - name: darling_core
    version: 0.20.11
    sha: 0d00b9596d185e565c2207a0b01f8bd1a135483d02d9b7b0a54b11da8d53412e
  - name: darling_macro
    version: 0.20.11
    sha: fc34b93ccb385b40dc71c6fceac4b2ad23662c7eeb248cf10d529b7e055b6ead
  - name: dash-mpd
    version: 0.18.3
    sha: b87091d8b44ea20f31b5d55219664a636ffadfba0954a0bcb945437e1522a584
  - name: deranged
    version: 0.4.0
    sha: 9c9e6a11ca8224451684bc0d7d5a7adbf8f2fd6887261a1cfc3c0432f9d4068e
  - name: displaydoc
    version: 0.2.5
    sha: 97369cbbc041bc366949bc74d34658d6cda5621039731c6310521892a3a20ae0
  - name: ebml-iterable
    version: 0.6.3
    sha: 7b5173ac3752f08b526a6991509615e1a345b221ec3c58c7633433e8c9582312
  - name: ebml-iterable-specification
    version: 0.4.0
    sha: f56467af159a98735d44231f53eaa505e919e6003266f103b99649a93f106784
  - name: ebml-iterable-specification-derive
    version: 0.4.0
    sha: b066b81018300fdce40f71c4db355a102699324af96fad28f25ab1b5f87de066
  - name: either
    version: 1.15.0
    sha: 48c757948c5ede0e46177b7add2e67155f70e33c07fea8284df6576da70b3719
  - name: equivalent
    version: 1.0.2
    sha: 877a4ace8713b0bcf2a4e7eec82529c029f1d0619886d18145fea96c3ffe5c0f
  - name: errno
    version: 0.3.11
    sha: 976dd42dc7e85965fe702eb8164f21f450704bdde31faefd6471dba214cb594e
  - name: fnv
    version: 1.0.7
    sha: 3f9eec918d3f24069decb9af1554cad7c880e2da24a9afd88aca000531ab82c1
  - name: form_urlencoded
    version: 1.2.1
    sha: e13624c2627564efccf4934284bdd98cbaa14e79b0b5a141218e507b3a823456
  - name: fs-err
    version: 3.1.0
    sha: 1f89bda4c2a21204059a977ed3bfe746677dfd137b83c339e702b0ac91d482aa
  - name: futures
    version: 0.3.31
    sha: 65bc07b1a8bc7c85c5f2e110c476c7389b4554ba72af57d8445ea63a576b0876
  - name: futures-channel
    version: 0.3.31
    sha: 2dff15bf788c671c1934e366d07e30c1814a8ef514e1af724a602e8a2fbe1b10
  - name: futures-core
    version: 0.3.31
    sha: 05f29059c0c2090612e8d742178b0580d2dc940c837851ad723096f87af6663e
  - name: futures-executor
    version: 0.3.31
    sha: 1e28d1d997f585e54aebc3f97d39e72338912123a67330d723fdbb564d646c9f
  - name: futures-io
    version: 0.3.31
    sha: 9e5c1b78ca4aae1ac06c48a526a655760685149f0d465d21f37abfe57ce075c6
  - name: futures-macro
    version: 0.3.31
    sha: 162ee34ebcb7c64a8abebc059ce0fee27c2262618d7b60ed8faf72fef13c3650
  - name: futures-sink
    version: 0.3.31
    sha: e575fab7d1e0dcb8d0c7bcf9a63ee213816ab51902e6d244a95819acacf1d4f7
  - name: futures-task
    version: 0.3.31
    sha: f90f7dce0722e95104fcb095585910c0977252f286e354b5e3bd38902cd99988
  - name: futures-util
    version: 0.3.31
    sha: 9fa08315bb612088cc391249efdc3bc77536f16c91f6cf495e6fbe85b20a4a81
  - name: gio-sys
    version: 0.20.10
    sha: 521e93a7e56fc89e84aea9a52cfc9436816a4b363b030260b699950ff1336c83
  - name: glib
    version: 0.20.10
    sha: c501c495842c2b23cdacead803a5a343ca2a5d7a7ddaff14cc5f6cf22cfb92c2
  - name: glib-macros
    version: 0.20.10
    sha: ebe6dc9ce29887c4b3b74d78d5ba473db160a258ae7ed883d23632ac7fed7bc9
  - name: glib-sys
    version: 0.20.10
    sha: 8ab79e1ed126803a8fb827e3de0e2ff95191912b8db65cee467edb56fc4cc215
  - name: gobject-sys
    version: 0.20.10
    sha: ec9aca94bb73989e3cfdbf8f2e0f1f6da04db4d291c431f444838925c4c63eda
  - name: gst-plugin-version-helper
    version: 0.8.2
    sha: 4e5e874f1660252fd2ec81c602066df3633b3a6fcbe2b196f7f93c27cf069b2a
  - name: gstreamer
    version: 0.23.6
    sha: 50ab4c88f731596a2511a6f14cabdd666e0d8efab62a1d58e6ddb57faa96e22e
  - name: gstreamer-app
    version: 0.23.5
    sha: 2e9a883eb21aebcf1289158225c05f7aea5da6ecf71fa7f0ff1ce4d25baf004e
  - name: gstreamer-app-sys
    version: 0.23.5
    sha: 94f7ef838306fe51852d503a14dc79ac42de005a59008a05098de3ecdaf05455
  - name: gstreamer-audio
    version: 0.23.6
    sha: 2e7ec7e0374298897e669db7c79544bc44df12011985e7dd5f38644edaf2caf4
  - name: gstreamer-audio-sys
    version: 0.23.6
    sha: 2b5f3e09e7c04ec91d78c2a6ca78d50b574b9ed49fdf5e72f3693adca4306a87
  - name: gstreamer-base
    version: 0.23.6
    sha: f19a74fd04ffdcb847dd322640f2cf520897129d00a7bcb92fd62a63f3e27404
  - name: gstreamer-base-sys
    version: 0.23.6
    sha: 87f2fb0037b6d3c5b51f60dea11e667910f33be222308ca5a101450018a09840
  - name: gstreamer-check
    version: 0.23.5
    sha: 60ee88620d2ef3d1ca218dedb40f39e759650a12704cf5976a95fa048a3d4f3e
  - name: gstreamer-check-sys
    version: 0.23.5
    sha: 2500c1e8af0bd17125bf2397b4b954f7f8fcc273f1b9545e1852fd5b8cc2bfeb
  - name: gstreamer-pbutils
    version: 0.23.5
    sha: acf4bf5857fa22f910634e86a5bce33b5581a9e90caa4e32fd4a20bdd4c83ed0
  - name: gstreamer-pbutils-sys
    version: 0.23.5
    sha: 304101f5fccbbe41e0169536777ddb7680c2c837e18575c22b30fc20cedfb76f
  - name: gstreamer-sys
    version: 0.23.6
    sha: feea73b4d92dbf9c24a203c9cd0bcc740d584f6b5960d5faf359febf288919b2
  - name: gstreamer-tag
    version: 0.23.6
    sha: b52aa41071bfd846faba7508c0a7ef0562aefc135517b2af8eb54c08c284f9c4
  - name: gstreamer-tag-sys
    version: 0.23.6
    sha: 19d1757fb8d77a2c4e6662f8a42672e5ddbde847b32d2db1775877e925626f72
  - name: gstreamer-video
    version: 0.23.6
    sha: 1318b599d77ca4f7702ecbdeac1672d6304cb16b7e5752fabb3ee8260449a666
  - name: gstreamer-video-sys
    version: 0.23.6
    sha: 0a70f0947f12d253b9de9bc3fd92f981e4d025336c18389c7f08cdf388a99f5c
  - name: hashbrown
    version: 0.12.3
    sha: 8a9ee70c43aaf417c914396645a0fa852624801b24ebb7ae78fe8272889ac888
  - name: hashbrown
    version: 0.15.3
    sha: 84b26c544d002229e640969970a2e74021aadf6e2f96372b9c58eff97de08eb3
  - name: heck
    version: 0.5.0
    sha: 2304e00983f87ffb38b55b444b5e3b60a884b5d30c0fca7d82fe33449bbe55ea
  - name: hex
    version: 0.4.3
    sha: 7f24254aa9a54b5c858eaee2f5bccdb46aaf0e486a595ed5fd8f86ba55232a70
  - name: hxdmp
    version: 0.2.1
    sha: a17b27f28a7466846baca75f0a5244e546e44178eb7f1c07a3820f413e91c6b0
  - name: iana-time-zone
    version: 0.1.63
    sha: b0c919e5debc312ad217002b8048a17b7d83f80703865bbfcfebb0458b0b27d8
  - name: iana-time-zone-haiku
    version: 0.1.2
    sha: f31827a206f56af32e590ba56d5d2d085f558508192593743f16b2306495269f
  - name: icu_collections
    version: 1.5.0
    sha: db2fa452206ebee18c4b5c2274dbf1de17008e874b4dc4f0aea9d01ca79e4526
  - name: icu_locid
    version: 1.5.0
    sha: 13acbb8371917fc971be86fc8057c41a64b521c184808a698c02acc242dbf637
  - name: icu_locid_transform
    version: 1.5.0
    sha: 01d11ac35de8e40fdeda00d9e1e9d92525f3f9d887cdd7aa81d727596788b54e
  - name: icu_locid_transform_data
    version: 1.5.1
    sha: 7515e6d781098bf9f7205ab3fc7e9709d34554ae0b21ddbcb5febfa4bc7df11d
  - name: icu_normalizer
    version: 1.5.0
    sha: 19ce3e0da2ec68599d193c93d088142efd7f9c5d6fc9b803774855747dc6a84f
  - name: icu_normalizer_data
    version: 1.5.1
    sha: c5e8338228bdc8ab83303f16b797e177953730f601a96c25d10cb3ab0daa0cb7
  - name: icu_properties
    version: 1.5.1
    sha: 93d6020766cfc6302c15dbbc9c8778c37e62c14427cb7f6e601d849e092aeef5
  - name: icu_properties_data
    version: 1.5.1
    sha: 85fb8799753b75aee8d2a21d7c14d9f38921b54b3dbda10f5a3c7a7b82dba5e2
  - name: icu_provider
    version: 1.5.0
    sha: 6ed421c8a8ef78d3e2dbc98a973be2f3770cb42b606e3ab18d6237c4dfde68d9
  - name: icu_provider_macros
    version: 1.5.0
    sha: 1ec89e9337638ecdc08744df490b221a7399bf8d164eb52a665454e60e075ad6
  - name: ident_case
    version: 1.0.1
    sha: b9e0384b61958566e926dc50660321d12159025e767c18e043daf26b70104c39
  - name: idna
    version: 1.0.3
    sha: 686f825264d630750a544639377bae737628043f20d38bbc029e8f29ea968a7e
  - name: idna_adapter
    version: 1.2.0
    sha: daca1df1c957320b2cf139ac61e7bd64fed304c5040df000a745aa1de3b4ef71
  - name: indexmap
    version: 1.9.3
    sha: bd070e393353796e801d209ad339e89596eb4c8d430d18ede6a1cced8fafbd99
  - name: indexmap
    version: 2.9.0
    sha: cea70ddb795996207ad57735b50c5982d8844f38ba9ee5f1aedcfb708a2aa11e
  - name: iso8601
    version: 0.6.2
    sha: c5c177cff824ab21a6f41079a4c401241c4e8be14f316c4c6b07d5fca351c98d
  - name: itertools
    version: 0.14.0
    sha: 2b192c782037fadd9cfa75548310488aabdbf3d2da73885b31bd0abd03351285
  - name: itoa
    version: 1.0.15
    sha: 4a5f13b858c8d314ee3e8f639011f7ccefe71f97f96e50151fb991f267928e2c
  - name: js-sys
    version: 0.3.77
    sha: 1cfaf33c695fc6e08064efbc1f72ec937429614f25eef83af942d0e227c3a28f
  - name: lazy_static
    version: 1.5.0
    sha: bbd2bcb4c963f2ddae06a2efc7e9f3591312473c50c6685e1f298068316e66fe
  - name: libc
    version: 0.2.172
    sha: d750af042f7ef4f724306de029d18836c26c1765a54a6a3f094cbd23a7267ffa
  - name: linux-raw-sys
    version: 0.9.4
    sha: cd945864f07fe9f5371a27ad7b52a172b4b499999f1d97574c9fa68373937e12
  - name: litemap
    version: 0.7.3
    sha: 643cb0b8d4fcc284004d5fd0d67ccf61dfffadb7f75e1e71bc420f4688a3a704
  - name: log
    version: 0.4.27
    sha: 13dc2df351e3202783a1fe0d44375f7295ffb4049267b0f3018346dc122a1d94
  - name: m3u8-rs
    version: 6.0.0
    sha: f03cd3335fb5f2447755d45cda9c70f76013626a9db44374973791b0926a86c3
  - name: memchr
    version: 2.7.4
    sha: 78ca9ab1a0babb1e7d5695e3530886289c18cf2f87ec19a575a0abdce112e3a3
  - name: minimal-lexical
    version: 0.2.1
    sha: 68354c5c6bd36d73ff3feceb05efa59b6acb7626617f4962be322a825e61f79a
  - name: muldiv
    version: 1.0.1
    sha: 956787520e75e9bd233246045d19f42fb73242759cc57fba9611d940ae96d4b0
  - name: nom
    version: 7.1.3
    sha: d273983c5a657a70a3e8f2a01329822f3b8c8172b73826411a55751e404a0a4a
  - name: nom
    version: 8.0.0
    sha: df9761775871bdef83bee530e60050f7e54b1105350d6884eb0fb4f46c2f9405
  - name: num-conv
    version: 0.1.0
    sha: 51d515d32fb182ee37cda2ccdcb92950d6a3c2893aa280e540671c2cd0f3b1d9
  - name: num-integer
    version: 0.1.46
    sha: 7969661fd2958a5cb096e56c8e1ad0444ac2bbcd0061bd28660485a44879858f
  - name: num-rational
    version: 0.4.2
    sha: f83d14da390562dca69fc84082e73e548e1ad308d24accdedd2720017cb37824
  - name: num-traits
    version: 0.2.19
    sha: 071dfc062690e90b734c0b2273ce72ad0ffa95f0c74596bc250dcfd960262841
  - name: once_cell
    version: 1.21.3
    sha: 42f5e15c9953c5e4ccceeb2e7382a716482c34515315f7b03532b8b4e8393d2d
  - name: option-operations
    version: 0.5.0
    sha: 7c26d27bb1aeab65138e4bf7666045169d1717febcc9ff870166be8348b223d0
  - name: paste
    version: 1.0.15
    sha: 57c0d7b74b563b49d38dae00a0c37d4d6de9b432382b2892f0574ddcae73fd0a
  - name: percent-encoding
    version: 2.3.1
    sha: e3148f5046208a5d56bcfc03053e3ca6334e51da8dfb19b6cdc8b306fae3283e
  - name: pin-project-lite
    version: 0.2.16
    sha: 3b3cff922bd51709b605d9ead9aa71031d81447142d828eb4a6eba76fe619f9b
  - name: pin-utils
    version: 0.1.0
    sha: 8b870d8c151b6f2fb93e84a13146138f05d02ed11c7e7c54f8826aaaf7c9f184
  - name: pkg-config
    version: 0.3.32
    sha: 7edddbd0b52d732b21ad9a5fab5c704c14cd949e5e9a1ec5929a24fded1b904c
  - name: powerfmt
    version: 0.2.0
    sha: 439ee305def115ba05938db6eb1644ff94165c5ab5e9420d1c1bcedbba909391
  - name: proc-macro-crate
    version: 3.3.0
    sha: edce586971a4dfaa28950c6f18ed55e0406c1ab88bbce2c6f6293a7aaba73d35
  - name: proc-macro2
    version: 1.0.95
    sha: 02b3e5e68a3a1a02aad3ec490a98007cbc13c37cbe84a3cd7b8e406d76e7f778
  - name: quick-xml
    version: 0.37.5
    sha: 331e97a1af0bf59823e6eadffe373d7b27f485be8748f71471c662c1f269b7fb
  - name: quote
    version: 1.0.40
    sha: 1885c039570dc00dcb4ff087a89e185fd56bae234ddc7f056a945bf36467248d
  - name: regex
    version: 1.11.1
    sha: b544ef1b4eac5dc2db33ea63606ae9ffcfac26c1416a2806ae0bf5f56b201191
  - name: regex-automata
    version: 0.4.9
    sha: 809e8dc61f6de73b46c85f4c96486310fe304c434cfa43669d7b40f711150908
  - name: regex-syntax
    version: 0.8.5
    sha: 2b15c43186be67a4fd63bee50d0303afffcef381492ebe2c5d87f324e1b8815c
  - name: rustix
    version: 1.0.7
    sha: c71e83d6afe7ff64890ec6b71d6a69bb8a610ab78ce364b3352876bb4c801266
  - name: rustversion
    version: 1.0.20
    sha: eded382c5f5f786b989652c49544c4877d9f015cc22e145a5ea8ea66c2921cd2
  - name: ryu
    version: 1.0.20
    sha: 28d3b2b1366ec20994f1fd18c3c594f05c5dd4bc44d8bb0c1c632c8d6829481f
  - name: serde
    version: 1.0.219
    sha: 5f0e2c6ed6606019b4e29e69dbaba95b11854410e5347d525002456dbbb786b6
  - name: serde_derive
    version: 1.0.219
    sha: 5b0276cf7f2c73365f7157c8123c21cd9a50fbbd844757af28ca1f5925fc2a00
  - name: serde_json
    version: 1.0.140
    sha: 20068b6e96dc6c9bd23e01df8827e6c7e1f2fddd43c21810382803c136b99373
  - name: serde_path_to_error
    version: 0.1.17
    sha: 59fab13f937fa393d08645bf3a84bdfe86e296747b506ada67bb15f10f218b2a
  - name: serde_spanned
    version: 0.6.8
    sha: 87607cb1398ed59d48732e575a4c28a7a8ebf2454b964fe3f224f2afc07909e1
  - name: serde_with
    version: 3.12.0
    sha: d6b6f7f2fcb69f747921f79f3926bd1e203fce4fef62c268dd3abfb6d86029aa
  - name: serde_with_macros
    version: 3.12.0
    sha: 8d00caa5193a3c8362ac2b73be6b9e768aa5a4b2f721d8f4b339600c3cb51f8e
  - name: shlex
    version: 1.3.0
    sha: 0fda2ff0d084019ba4d7c6f371c95d8fd75ce3524c3cb8fb653a3023f6323e64
  - name: slab
    version: 0.4.9
    sha: 8f92a496fb766b417c996b9c5e57daf2f7ad3b0bebe1ccfca4856390e3d3bb67
  - name: smallvec
    version: 1.15.0
    sha: 8917285742e9f3e1683f0a9c4e6b57960b7314d0b08d30d1ecd426713ee2eee9
  - name: stable_deref_trait
    version: 1.2.0
    sha: a8f112729512f8e442d81f95a8a7ddf2b7c6b8a1a6f509a95864142b30cab2d3
  - name: strsim
    version: 0.11.1
    sha: 7da8b5736845d9f2fcb837ea5d9e2628564b3b043a70948a3f0b778838c5fb4f
  - name: syn
    version: 1.0.109
    sha: 72b64191b275b66ffe2469e8af2c1cfe3bafa67b529ead792a6d0160888b4237
  - name: syn
    version: 2.0.101
    sha: 8ce2b7fc941b3a24138a0a7cf8e858bfc6a992e7978a068a5c760deb0ed43caf
  - name: synstructure
    version: 0.13.2
    sha: 728a70f3dbaf5bab7f0c4b1ac8d7ae5ea60a4b5549c8a5914361c99147a709d2
  - name: system-deps
    version: 7.0.3
    sha: 66d23aaf9f331227789a99e8de4c91bf46703add012bdfd45fdecdfb2975a005
  - name: target-lexicon
    version: 0.12.16
    sha: 61c41af27dd6d1e27b1b16b489db798443478cef1f06a660c96db617ba5de3b1
  - name: thiserror
    version: 2.0.12
    sha: 567b8a2dae586314f7be2a752ec7474332959c6460e02bde30d702a66d488708
  - name: thiserror-impl
    version: 2.0.12
    sha: 7f7cf42b4507d8ea322120659672cf1b9dbb93f8f2d4ecfd6e51350ff5b17a1d
  - name: time
    version: 0.3.41
    sha: 8a7619e19bc266e0f9c5e6686659d394bc57973859340060a69221e57dbc0c40
  - name: time-core
    version: 0.1.4
    sha: c9e9a38711f559d9e3ce1cdb06dd7c5b8ea546bc90052da6d06bb76da74bb07c
  - name: time-macros
    version: 0.2.22
    sha: 3526739392ec93fd8b359c8e98514cb3e8e021beb4e5f597b00a0221f8ed8a49
  - name: tinystr
    version: 0.7.6
    sha: 9117f5d4db391c1cf6927e7bea3db74b9a1c1add8f7eda9ffd5364f40f57b82f
  - name: toml
    version: 0.8.22
    sha: 05ae329d1f08c4d17a59bed7ff5b5a769d062e64a62d34a3261b219e62cd5aae
  - name: toml_datetime
    version: 0.6.9
    sha: 3da5db5a963e24bc68be8b17b6fa82814bb22ee8660f192bb182771d498f09a3
  - name: toml_edit
    version: 0.22.26
    sha: 310068873db2c5b3e7659d2cc35d21855dbafa50d1ce336397c666e3cb08137e
  - name: tracing
    version: 0.1.41
    sha: 784e0ac535deb450455cbfa28a6f0df145ea1bb7ae51b821cf5e7927fdcfbdd0
  - name: tracing-attributes
    version: 0.1.28
    sha: 395ae124c09f9e6918a2310af6038fba074bcf474ac352496d5910dd59a2226d
  - name: tracing-core
    version: 0.1.33
    sha: e672c95779cf947c5311f83787af4fa8fffd12fb27e4993211a84bdfd9610f9c
  - name: unicode-ident
    version: 1.0.18
    sha: 5a5f39404a5da50712a4c1eecf25e90dd62b613502b7e925fd4e4d19b5c96512
  - name: url
    version: 2.5.4
    sha: 32f8b686cadd1473f4bd0117a5d28d36b1ade384ea9b5069a1c40aefed7fda60
  - name: utf16_iter
    version: 1.0.5
    sha: c8232dd3cdaed5356e0f716d285e4b40b932ac434100fe9b7e0e8e935b9e6246
  - name: utf8_iter
    version: 1.0.4
    sha: b6c140620e7ffbb22c2dee59cafe6084a59b5ffc27a8859a5f0d494b5d52b6be
  - name: version-compare
    version: 0.2.0
    sha: 852e951cb7832cb45cb1169900d19760cfa39b82bc0ea9c0e5a14ae88411c98b
  - name: wasm-bindgen
    version: 0.2.100
    sha: 1edc8929d7499fc4e8f0be2262a241556cfc54a0bea223790e71446f2aab1ef5
  - name: wasm-bindgen-backend
    version: 0.2.100
    sha: 2f0a0651a5c2bc21487bde11ee802ccaf4c51935d0d3d42a6101f98161700bc6
  - name: wasm-bindgen-macro
    version: 0.2.100
    sha: 7fe63fc6d09ed3792bd0897b314f53de8e16568c2b3f7982f468c0bf9bd0b407
  - name: wasm-bindgen-macro-support
    version: 0.2.100
    sha: 8ae87ea40c9f689fc23f209965b6fb8a99ad69aeeb0231408be24920604395de
  - name: wasm-bindgen-shared
    version: 0.2.100
    sha: 1a05d73b933a847d6cccdda8f838a22ff101ad9bf93e33684f39c1f5f0eece3d
  - name: webm-iterable
    version: 0.6.4
    sha: cd9fbf173b4b38f2f8bbb0082a0d4cb21f263a70811f5fccb1663c421c66d9f9
  - name: windows-core
    version: 0.61.0
    sha: 4763c1de310c86d75a878046489e2e5ba02c649d185f21c67d4cf8a56d098980
  - name: windows-implement
    version: 0.60.0
    sha: a47fddd13af08290e67f4acabf4b459f647552718f683a7b415d290ac744a836
  - name: windows-interface
    version: 0.59.1
    sha: bd9211b69f8dcdfa817bfd14bf1c97c9188afa36f4750130fcdf3f400eca9fa8
  - name: windows-link
    version: 0.1.1
    sha: 76840935b766e1b0a05c0066835fb9ec80071d4c09a16f6bd5f7e655e3c14c38
  - name: windows-result
    version: 0.3.2
    sha: c64fd11a4fd95df68efcfee5f44a294fe71b8bc6a91993e2791938abcc712252
  - name: windows-strings
    version: 0.4.0
    sha: 7a2ba9642430ee452d5a7aa78d72907ebe8cfda358e8cb7918a2050581322f97
  - name: windows-sys
    version: 0.52.0
    sha: 282be5f36a8ce781fad8c8ae18fa3f9beff57ec1b52cb3de0789201425d9a33d
  - name: windows-sys
    version: 0.59.0
    sha: 1e38bc4d79ed67fd075bcc251a1c39b32a1776bbe92e5bef1f0bf1f8c531853b
  - name: windows-targets
    version: 0.52.6
    sha: 9b724f72796e036ab90c1021d4780d4d3d648aca59e491e6b98e725b84e99973
  - name: windows_aarch64_gnullvm
    version: 0.52.6
    sha: 32a4622180e7a0ec044bb555404c800bc9fd9ec262ec147edd5989ccd0c02cd3
  - name: windows_aarch64_msvc
    version: 0.52.6
    sha: 09ec2a7bb152e2252b53fa7803150007879548bc709c039df7627cabbd05d469
  - name: windows_i686_gnu
    version: 0.52.6
    sha: 8e9b5ad5ab802e97eb8e295ac6720e509ee4c243f69d781394014ebfe8bbfa0b
  - name: windows_i686_gnullvm
    version: 0.52.6
    sha: 0eee52d38c090b3caa76c563b86c3a4bd71ef1a819287c19d586d7334ae8ed66
  - name: windows_i686_msvc
    version: 0.52.6
    sha: 240948bc05c5e7c6dabba28bf89d89ffce3e303022809e73deaefe4f6ec56c66
  - name: windows_x86_64_gnu
    version: 0.52.6
    sha: 147a5c80aabfbf0c7d901cb5895d1de30ef2907eb21fbbab29ca94c5b08b1a78
  - name: windows_x86_64_gnullvm
    version: 0.52.6
    sha: 24d5b23dc417412679681396f2b49f3de8c1473deb516bd34410872eff51ed0d
  - name: windows_x86_64_msvc
    version: 0.52.6
    sha: 589f6da84c646204747d1270a2a5661ea66ed1cced2631d546fdfb155959f9ec
  - name: winnow
    version: 0.7.10
    sha: c06928c8748d81b05c9be96aad92e1b6ff01833332f281e8cfca3be4b35fc9ec
  - name: write16
    version: 1.0.0
    sha: d1890f4022759daae28ed4fe62859b1236caebfc61ede2f63ed4e695f3f6d936
  - name: writeable
    version: 0.5.5
    sha: 1e9df38ee2d2c3c5948ea468a8406ff0db0b29ae1ffde1bcf20ef305bcc95c51
  - name: xattr
    version: 1.5.0
    sha: 0d65cbf2f12c15564212d48f4e3dfb87923d25d611f2aed18f4cb23f0413d89e
  - name: yoke
    version: 0.7.5
    sha: 120e6aef9aa629e3d4f52dc8cc43a015c7724194c97dfaf45180d2daf2b77f40
  - name: yoke-derive
    version: 0.7.5
    sha: 2380878cad4ac9aac1e2435f3eb4020e8374b5f13c296cb75b4620ff8e229154
  - name: zerofrom
    version: 0.1.4
    sha: 91ec111ce797d0e0784a1116d0ddcdbea84322cd79e5d5ad173daeba4f93ab55
  - name: zerofrom-derive
    version: 0.1.4
    sha: 0ea7b4a3637ea8669cedf0f1fd5c286a17f3de97b8dd5a70a6c167a1730e63a5
  - name: zerovec
    version: 0.10.4
    sha: aa2b893d79df23bfb12d5461018d408ea19dfafe76c2c7ef6d4eba614f8ff079
  - name: zerovec-derive
    version: 0.10.3
    sha: 6eafa6dfb17584ea3e2bd6e76e0cc15ad7af12b09abdd1ca55961bed9b1063c6
