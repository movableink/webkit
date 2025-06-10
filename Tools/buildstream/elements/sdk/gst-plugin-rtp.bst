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
      PAGER=this-is-not-a-pager gst-inspect-1.0 rsrtp

sources:
- kind: tar
  url: crates:crates/gst-plugin-rtp/gst-plugin-rtp-0.13.5.crate
  ref: be4232834a935ab75e8e9e0003ca8400f3a35b71a2abd9a6464c16a0147c651a
- kind: local
  path: files/gst-plugin-rtp/Cargo.lock
- kind: cargo
  ref:
  - name: addr2line
    version: 0.24.2
    sha: dfbe277e56a376000877090da837660b4427aad530e3028d44e0bffe4f89a1c1
  - name: adler2
    version: 2.0.0
    sha: 512761e0bb2578dd7380c6baaa0f4ce03e84f95e960231d1dec8bf4d7d6e2627
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
  - name: backtrace
    version: 0.3.74
    sha: 8d82cb332cdfaed17ae235a638438ac4d4839913cc2af585c3c6746e8f8bee1a
  - name: bitflags
    version: 2.9.0
    sha: 5c8214115b7bf84099f1309324e63141d4c5d7cc26862f97a0a857dbefe165bd
  - name: bitstream-io
    version: 2.3.0
    sha: 7c12d1856e42f0d817a835fe55853957c85c8c8a470114029143d3f12671446e
  - name: bumpalo
    version: 3.17.0
    sha: 1628fb46dfa0b37568d12e5edd512553eccf6a22a78e8bde00bb4aed84d5bdbf
  - name: byte-slice-cast
    version: 1.2.3
    sha: 7575182f7272186991736b70173b0ea045398f984bf5ebbb3804736ce1330c9d
  - name: cc
    version: 1.2.21
    sha: 8691782945451c1c383942c4874dbe63814f61cb57ef773cda2972682b7bb3c0
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
  - name: deranged
    version: 0.4.0
    sha: 9c9e6a11ca8224451684bc0d7d5a7adbf8f2fd6887261a1cfc3c0432f9d4068e
  - name: either
    version: 1.15.0
    sha: 48c757948c5ede0e46177b7add2e67155f70e33c07fea8284df6576da70b3719
  - name: equivalent
    version: 1.0.2
    sha: 877a4ace8713b0bcf2a4e7eec82529c029f1d0619886d18145fea96c3ffe5c0f
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
  - name: getrandom
    version: 0.3.2
    sha: 73fea8450eea4bac3940448fb7ae50d91f034f941199fcd9d909a5a07aa455f0
  - name: gimli
    version: 0.31.1
    sha: 07e28edb80900c19c28f1072f2e8aeca7fa06b23cd4169cefe1af5aa3260783f
  - name: gio
    version: 0.20.9
    sha: a4f00c70f8029d84ea7572dd0e1aaa79e5329667b4c17f329d79ffb1e6277487
  - name: gio-sys
    version: 0.20.9
    sha: 160eb5250a26998c3e1b54e6a3d4ea15c6c7762a6062a19a7b63eff6e2b33f9e
  - name: glib
    version: 0.20.9
    sha: 707b819af8059ee5395a2de9f2317d87a53dbad8846a2f089f0bb44703f37686
  - name: glib-macros
    version: 0.20.7
    sha: 715601f8f02e71baef9c1f94a657a9a77c192aea6097cf9ae7e5e177cd8cde68
  - name: glib-sys
    version: 0.20.9
    sha: a8928869a44cfdd1fccb17d6746e4ff82c8f82e41ce705aa026a52ca8dc3aefb
  - name: gobject-sys
    version: 0.20.9
    sha: c773a3cb38a419ad9c26c81d177d96b4b08980e8bdbbf32dace883e96e96e7e3
  - name: gst-plugin-version-helper
    version: 0.8.2
    sha: 4e5e874f1660252fd2ec81c602066df3633b3a6fcbe2b196f7f93c27cf069b2a
  - name: gstreamer
    version: 0.23.5
    sha: 2188fe829b0ebe12e4cf2bbcf6658470a936269daba7afae92847a2af32c9105
  - name: gstreamer-app
    version: 0.23.5
    sha: 2e9a883eb21aebcf1289158225c05f7aea5da6ecf71fa7f0ff1ce4d25baf004e
  - name: gstreamer-app-sys
    version: 0.23.5
    sha: 94f7ef838306fe51852d503a14dc79ac42de005a59008a05098de3ecdaf05455
  - name: gstreamer-audio
    version: 0.23.5
    sha: 49118ca684e2fc42207509fcac8497d91079c2ffe8ff2b4ae99e71dbafef1ede
  - name: gstreamer-audio-sys
    version: 0.23.5
    sha: 7d469526ecf30811b50a6460fd285ee40d189c46048b3d0c69b67a04b414fb51
  - name: gstreamer-base
    version: 0.23.5
    sha: ad33dd444db0d215ac363164f900f800ffb93361ad8a60840e95e14b7de985e8
  - name: gstreamer-base-sys
    version: 0.23.5
    sha: 114b2a704f19a70f20c54b00e54f5d5376bbf78bd2791e6beb0776c997d8bf24
  - name: gstreamer-check
    version: 0.23.5
    sha: 60ee88620d2ef3d1ca218dedb40f39e759650a12704cf5976a95fa048a3d4f3e
  - name: gstreamer-check-sys
    version: 0.23.5
    sha: 2500c1e8af0bd17125bf2397b4b954f7f8fcc273f1b9545e1852fd5b8cc2bfeb
  - name: gstreamer-net
    version: 0.23.5
    sha: f7a57013f1af740373c5cbf1602b6ff25238d4a0e300eff179b75676c9c7781e
  - name: gstreamer-net-sys
    version: 0.23.5
    sha: 6b0291a3064e6e94d640f2d662264b726e5e0c2067ceb17f2b56dec6f6920877
  - name: gstreamer-rtp
    version: 0.23.5
    sha: bf35002f6eff177f5afc255fa7a2e7ab718fe8186a737dfdb791784e63571154
  - name: gstreamer-rtp-sys
    version: 0.23.5
    sha: 2b874248d40bd6678be9c7218ec880735258cf2ba77000c72efda8bb3271b0fe
  - name: gstreamer-sys
    version: 0.23.5
    sha: fe159238834058725808cf6604a7c5d9e4a50e1eacd7b0c63bce2fe3a067dbd1
  - name: gstreamer-video
    version: 0.23.5
    sha: ad242d388b63c91652c8157de3b0c1f709e49c941a0aae1952455f6ee326ca2d
  - name: gstreamer-video-sys
    version: 0.23.5
    sha: 465ff496889fb38be47f5e821163c2e83414d87c4aa55f5aae62dc7200971d4d
  - name: hashbrown
    version: 0.15.3
    sha: 84b26c544d002229e640969970a2e74021aadf6e2f96372b9c58eff97de08eb3
  - name: heck
    version: 0.5.0
    sha: 2304e00983f87ffb38b55b444b5e3b60a884b5d30c0fca7d82fe33449bbe55ea
  - name: hex
    version: 0.4.3
    sha: 7f24254aa9a54b5c858eaee2f5bccdb46aaf0e486a595ed5fd8f86ba55232a70
  - name: iana-time-zone
    version: 0.1.63
    sha: b0c919e5debc312ad217002b8048a17b7d83f80703865bbfcfebb0458b0b27d8
  - name: iana-time-zone-haiku
    version: 0.1.2
    sha: f31827a206f56af32e590ba56d5d2d085f558508192593743f16b2306495269f
  - name: indexmap
    version: 2.9.0
    sha: cea70ddb795996207ad57735b50c5982d8844f38ba9ee5f1aedcfb708a2aa11e
  - name: itertools
    version: 0.13.0
    sha: 413ee7dfc52ee1a4949ceeb7dbc8a33f2d6c088194d9f922fb8318faf1f01186
  - name: js-sys
    version: 0.3.77
    sha: 1cfaf33c695fc6e08064efbc1f72ec937429614f25eef83af942d0e227c3a28f
  - name: libc
    version: 0.2.172
    sha: d750af042f7ef4f724306de029d18836c26c1765a54a6a3f094cbd23a7267ffa
  - name: log
    version: 0.4.27
    sha: 13dc2df351e3202783a1fe0d44375f7295ffb4049267b0f3018346dc122a1d94
  - name: memchr
    version: 2.7.4
    sha: 78ca9ab1a0babb1e7d5695e3530886289c18cf2f87ec19a575a0abdce112e3a3
  - name: miniz_oxide
    version: 0.8.8
    sha: 3be647b768db090acb35d5ec5db2b0e1f1de11133ca123b9eacf5137868f892a
  - name: muldiv
    version: 1.0.1
    sha: 956787520e75e9bd233246045d19f42fb73242759cc57fba9611d940ae96d4b0
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
  - name: object
    version: 0.36.7
    sha: 62948e14d923ea95ea2c7c86c71013138b66525b86bdc08d2dcc262bdb497b87
  - name: once_cell
    version: 1.21.3
    sha: 42f5e15c9953c5e4ccceeb2e7382a716482c34515315f7b03532b8b4e8393d2d
  - name: option-operations
    version: 0.5.0
    sha: 7c26d27bb1aeab65138e4bf7666045169d1717febcc9ff870166be8348b223d0
  - name: paste
    version: 1.0.15
    sha: 57c0d7b74b563b49d38dae00a0c37d4d6de9b432382b2892f0574ddcae73fd0a
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
  - name: ppv-lite86
    version: 0.2.21
    sha: 85eae3c4ed2f50dcfe72643da4befc30deadb458a9b590d720cde2f2b1e97da9
  - name: proc-macro-crate
    version: 3.3.0
    sha: edce586971a4dfaa28950c6f18ed55e0406c1ab88bbce2c6f6293a7aaba73d35
  - name: proc-macro2
    version: 1.0.95
    sha: 02b3e5e68a3a1a02aad3ec490a98007cbc13c37cbe84a3cd7b8e406d76e7f778
  - name: quote
    version: 1.0.40
    sha: 1885c039570dc00dcb4ff087a89e185fd56bae234ddc7f056a945bf36467248d
  - name: r-efi
    version: 5.2.0
    sha: 74765f6d916ee2faa39bc8e68e4f3ed8949b48cccdac59983d287a7cb71ce9c5
  - name: rand
    version: 0.9.1
    sha: 9fbfd9d094a40bf3ae768db9361049ace4c0e04a4fd6b359518bd7b73a73dd97
  - name: rand_chacha
    version: 0.9.0
    sha: d3022b5f1df60f26e1ffddd6c66e8aa15de382ae63b3a0c1bfc0e4d3e3f325cb
  - name: rand_core
    version: 0.9.3
    sha: 99d9a13982dcf210057a8a78572b2217b667c3beacbf3a0d8b454f6f82837d38
  - name: rtcp-types
    version: 0.1.0
    sha: ce5248489db464de29835170cd1f6e19933146b0016789effc59cb53d9f13844
  - name: rtp-types
    version: 0.1.2
    sha: 7bb90df8268abfe08452ef2dae9e867a54edfdaa71b3127ef47d8b031f77ac73
  - name: rustc-demangle
    version: 0.1.24
    sha: 719b953e2095829ee67db738b3bfa9fa368c94900df327b3f07fe6e794d2fe1f
  - name: rustversion
    version: 1.0.20
    sha: eded382c5f5f786b989652c49544c4877d9f015cc22e145a5ea8ea66c2921cd2
  - name: serde
    version: 1.0.219
    sha: 5f0e2c6ed6606019b4e29e69dbaba95b11854410e5347d525002456dbbb786b6
  - name: serde_derive
    version: 1.0.219
    sha: 5b0276cf7f2c73365f7157c8123c21cd9a50fbbd844757af28ca1f5925fc2a00
  - name: serde_spanned
    version: 0.6.8
    sha: 87607cb1398ed59d48732e575a4c28a7a8ebf2454b964fe3f224f2afc07909e1
  - name: shlex
    version: 1.3.0
    sha: 0fda2ff0d084019ba4d7c6f371c95d8fd75ce3524c3cb8fb653a3023f6323e64
  - name: slab
    version: 0.4.9
    sha: 8f92a496fb766b417c996b9c5e57daf2f7ad3b0bebe1ccfca4856390e3d3bb67
  - name: smallvec
    version: 1.15.0
    sha: 8917285742e9f3e1683f0a9c4e6b57960b7314d0b08d30d1ecd426713ee2eee9
  - name: syn
    version: 2.0.101
    sha: 8ce2b7fc941b3a24138a0a7cf8e858bfc6a992e7978a068a5c760deb0ed43caf
  - name: system-deps
    version: 7.0.3
    sha: 66d23aaf9f331227789a99e8de4c91bf46703add012bdfd45fdecdfb2975a005
  - name: target-lexicon
    version: 0.12.16
    sha: 61c41af27dd6d1e27b1b16b489db798443478cef1f06a660c96db617ba5de3b1
  - name: thiserror
    version: 1.0.69
    sha: b6aaf5339b578ea85b50e080feb250a3e8ae8cfcdff9a461c9ec2904bc923f52
  - name: thiserror
    version: 2.0.12
    sha: 567b8a2dae586314f7be2a752ec7474332959c6460e02bde30d702a66d488708
  - name: thiserror-impl
    version: 1.0.69
    sha: 4fee6c4efc90059e10f81e6d42c60a18f76588c3d74cb83a0b242a2b6c7504c1
  - name: thiserror-impl
    version: 2.0.12
    sha: 7f7cf42b4507d8ea322120659672cf1b9dbb93f8f2d4ecfd6e51350ff5b17a1d
  - name: time
    version: 0.3.41
    sha: 8a7619e19bc266e0f9c5e6686659d394bc57973859340060a69221e57dbc0c40
  - name: time-core
    version: 0.1.4
    sha: c9e9a38711f559d9e3ce1cdb06dd7c5b8ea546bc90052da6d06bb76da74bb07c
  - name: tokio
    version: 1.44.2
    sha: e6b88822cbe49de4185e3a4cbf8321dd487cf5fe0c5c65695fef6346371e9c48
  - name: toml
    version: 0.8.22
    sha: 05ae329d1f08c4d17a59bed7ff5b5a769d062e64a62d34a3261b219e62cd5aae
  - name: toml_datetime
    version: 0.6.9
    sha: 3da5db5a963e24bc68be8b17b6fa82814bb22ee8660f192bb182771d498f09a3
  - name: toml_edit
    version: 0.22.26
    sha: 310068873db2c5b3e7659d2cc35d21855dbafa50d1ce336397c666e3cb08137e
  - name: unicode-ident
    version: 1.0.18
    sha: 5a5f39404a5da50712a4c1eecf25e90dd62b613502b7e925fd4e4d19b5c96512
  - name: version-compare
    version: 0.2.0
    sha: 852e951cb7832cb45cb1169900d19760cfa39b82bc0ea9c0e5a14ae88411c98b
  - name: wasi
    version: 0.14.2+wasi-0.2.4
    sha: 9683f9a5a998d873c0d21fcbe3c083009670149a8fab228644b8bd36b2c48cb3
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
    version: 0.7.8
    sha: 9e27d6ad3dac991091e4d35de9ba2d2d00647c5d0fc26c5496dee55984ae111b
  - name: wit-bindgen-rt
    version: 0.39.0
    sha: 6f42320e61fe2cfd34354ecb597f86f413484a798ba44a8ca1165c58d42da6c1
  - name: zerocopy
    version: 0.8.25
    sha: a1702d9583232ddb9174e01bb7c15a2ab8fb1bc6f227aa1233858c351a3ba0cb
  - name: zerocopy-derive
    version: 0.8.25
    sha: 28a6e20d751156648aa063f3800b706ee209a32c0b4d9f24be3d980b01be55ef
