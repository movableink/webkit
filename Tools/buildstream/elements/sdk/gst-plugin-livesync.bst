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
      PAGER=this-is-not-a-pager gst-inspect-1.0 livesync

sources:
- kind: tar
  url: crates:crates/gst-plugin-livesync/gst-plugin-livesync-0.13.1.crate
  ref: 76dbe63a512f3d9a60ac586ee79055128f971baa6a4283e608735bd8df7972e8
- kind: cargo
  ref:
  - name: android-tzdata
    version: 0.1.1
    sha: e999941b234f3131b00bc13c22d06e8c5ff726d1b6318ac7eb276997bbb4fef0
  - name: android_system_properties
    version: 0.1.5
    sha: 819e7219dbd41043ac279b19830f2efc897156490d7fd6ea916720117ee66311
  - name: async-channel
    version: 2.3.1
    sha: 89b47800b0be77592da0afd425cc03468052844aff33b84e33cc696f64e77b6a
  - name: atomic_refcell
    version: 0.1.13
    sha: 41e67cd8309bbd06cd603a9e693a784ac2e5d1e955f11286e355089fcab3047c
  - name: autocfg
    version: 1.3.0
    sha: 0c4b4d0bd25bd0b74681c0ad21497610ce1b7c91b1022cd21c80c6fbdd9476b0
  - name: bitflags
    version: 2.6.0
    sha: b048fb63fd8b5923fc5aa7b340d8e156aec7ec02f0c78fa8a6ddc2613f6f71de
  - name: bumpalo
    version: 3.16.0
    sha: 79296716171880943b8470b5f8d03aa55eb2e645a4874bdbb28adb49162e012c
  - name: cairo-rs
    version: 0.20.1
    sha: e8a0ea147c94108c9613235388f540e4d14c327f7081c9e471fc8ee8a2533e69
  - name: cairo-sys-rs
    version: 0.20.0
    sha: 428290f914b9b86089f60f5d8a9f6e440508e1bcff23b25afd51502b0a2da88f
  - name: cc
    version: 1.1.15
    sha: 57b6a275aa2903740dc87da01c62040406b8812552e97129a63ea8850a17c6e6
  - name: cfg-expr
    version: 0.16.0
    sha: 345c78335be0624ed29012dc10c49102196c6882c12dde65d9f35b02da2aada8
  - name: cfg-if
    version: 1.0.0
    sha: baf1de4339761588bc0619e3cbc0120ee582ebb74b53b4efbf79117bd2da40fd
  - name: chrono
    version: 0.4.38
    sha: a21f936df1771bf62b77f047b726c4625ff2e8aa607c01ec06e5a05bd8463401
  - name: concurrent-queue
    version: 2.5.0
    sha: 4ca0197aee26d1ae37445ee532fefce43251d24cc7c166799f4d46817f1d3973
  - name: core-foundation-sys
    version: 0.8.7
    sha: 773648b94d0e5d620f64f280777445740e61fe701025087ec8b57f45c791888b
  - name: crossbeam-utils
    version: 0.8.20
    sha: 22ec99545bb0ed0ea7bb9b8e1e9122ea386ff8a48c0922e43f36d45ab09e0e80
  - name: either
    version: 1.13.0
    sha: 60b1af1c220855b6ceac025d3f6ecdd2b7c4894bfe9cd9bda4fbb4bc7c0d4cf0
  - name: equivalent
    version: 1.0.1
    sha: 5443807d6dff69373d433ab9ef5378ad8df50ca6298caf15de6e52e24aaf54d5
  - name: event-listener
    version: 5.3.1
    sha: 6032be9bd27023a771701cc49f9f053c751055f71efb2e0ae5c15809093675ba
  - name: event-listener-strategy
    version: 0.5.2
    sha: 0f214dc438f977e6d4e3500aaa277f5ad94ca83fbbd9b1a15713ce2344ccc5a1
  - name: field-offset
    version: 0.3.6
    sha: 38e2275cc4e4fc009b0669731a1e5ab7ebf11f469eaede2bab9309a5b4d6057f
  - name: futures-channel
    version: 0.3.30
    sha: eac8f7d7865dcb88bd4373ab671c8cf4508703796caa2b1985a9ca867b3fcb78
  - name: futures-core
    version: 0.3.30
    sha: dfc6580bb841c5a68e9ef15c77ccc837b40a7504914d52e47b8b0e9bbda25a1d
  - name: futures-executor
    version: 0.3.30
    sha: a576fc72ae164fca6b9db127eaa9a9dda0d61316034f33a0a0d4eda41f02b01d
  - name: futures-io
    version: 0.3.30
    sha: a44623e20b9681a318efdd71c299b6b222ed6f231972bfe2f224ebad6311f0c1
  - name: futures-macro
    version: 0.3.30
    sha: 87750cf4b7a4c0625b1529e4c543c2182106e4dedc60a2a6455e00d212c489ac
  - name: futures-task
    version: 0.3.30
    sha: 38d84fa142264698cdce1a9f9172cf383a0c82de1bddcf3092901442c4097004
  - name: futures-util
    version: 0.3.30
    sha: 3d6401deb83407ab3da39eba7e33987a73c3df0c82b4bb5813ee871c19c41d48
  - name: gdk-pixbuf
    version: 0.20.1
    sha: 8730751991b97419fc3f0c2dca2c9e45b48edf46e48e0f965964ecf33889812f
  - name: gdk-pixbuf-sys
    version: 0.20.1
    sha: 1ffbf649fd5b1c8c0f0feeb015b7533c3ef92da2887fb95ddd338bc2b1644a7c
  - name: gdk4
    version: 0.9.0
    sha: 4b7d7237c1487ed4b300aac7744efcbf1319e12d60d7afcd6f505414bd5b5dea
  - name: gdk4-sys
    version: 0.9.0
    sha: a67576c8ec012156d7f680e201a807b4432a77babb3157e0555e990ab6bcd878
  - name: gdk4-win32
    version: 0.9.0
    sha: 05afbcc8165b10f9143d2c4e4a589fce3a5071c6b20d75dd40704266d6eb4bb4
  - name: gdk4-win32-sys
    version: 0.9.0
    sha: 198c0604ceb45732c76b1fdb696d92b99400ea3a1c549fc5e40f4d79f8033205
  - name: gio
    version: 0.20.1
    sha: dcacaa37401cad0a95aadd266bc39c72a131d454fc012f6dfd217f891d76cc52
  - name: gio-sys
    version: 0.20.1
    sha: 5237611e97e9b86ab5768adc3eef853ae713ea797aa3835404acdfacffc9fb38
  - name: glib
    version: 0.20.2
    sha: 9c1ea829497810f8e87f5ee6d05c4879af641704add879e6b6080607cceeefe4
  - name: glib-macros
    version: 0.20.2
    sha: 951aa19c5e89555c0ca5e94ee874b24b2594ece8412b387bd84ee3266b8a3ea0
  - name: glib-sys
    version: 0.20.2
    sha: 92eee4531c1c9abba945d19378b205031b5890e1f99c319ba0503b6e0c06a163
  - name: gobject-sys
    version: 0.20.1
    sha: fa3d1dcd8a1eb2e7c22be3d5e792b14b186f3524f79b25631730f9a8c169d49a
  - name: graphene-rs
    version: 0.20.1
    sha: 80aac87f74e81c0e13433e892a047237abdc37945c86887f5eed905038356e69
  - name: graphene-sys
    version: 0.20.1
    sha: cc2f91ecd32989efad60326cc20a8fb252bd2852239a08e4e70cde8c100de9ca
  - name: gsk4
    version: 0.9.0
    sha: 1f3cf2091e1af185b347b3450817d93dea6fe435df7abd4c2cd7fb5bcb4cfda8
  - name: gsk4-sys
    version: 0.9.0
    sha: 6aa69614a26d8760c186c3690f1b0fbb917572ca23ef83137445770ceddf8cde
  - name: gst-plugin-gtk4
    version: 0.13.0
    sha: 69b98aeb531b6cf9c3a6eb36742cedc0168054e2f040b76ddc61260c10dec589
  - name: gst-plugin-version-helper
    version: 0.8.2
    sha: 4e5e874f1660252fd2ec81c602066df3633b3a6fcbe2b196f7f93c27cf069b2a
  - name: gstreamer
    version: 0.23.1
    sha: 683e15f8cc3a1a2646d9fe2181a58b7abb4c166256d8d15cce368e420c741140
  - name: gstreamer-audio
    version: 0.23.0
    sha: 50c434cbdde750d82853a978a59eb96e270c238d721a41bee0f37bfdc771f997
  - name: gstreamer-audio-sys
    version: 0.23.0
    sha: 4bb0542026fb062c2880d15e712c0814336e3209a054da43a64c14f7570da02f
  - name: gstreamer-base
    version: 0.23.1
    sha: ed5d73cb5cbf229c8904fba5ff93b1863f186bccc062064c1b2a9000750cc06e
  - name: gstreamer-base-sys
    version: 0.23.0
    sha: 4a6643ef963c636b8022adc93aa19eac6f356bd174a187c499339fc5d64c1e05
  - name: gstreamer-check
    version: 0.23.0
    sha: 6ab79cb5e46d8ff194088ea6438de603cf3598843c981bfa0186f7656dad09e1
  - name: gstreamer-check-sys
    version: 0.23.0
    sha: 40cd30046385250042032f2dea9b93a039411f58ee69e3b608f7c2f23e280abc
  - name: gstreamer-gl
    version: 0.23.0
    sha: cfe28e4b7c72022958add8a3d86e7293ff227a5086c426731b5ec5a15ffbc759
  - name: gstreamer-gl-sys
    version: 0.23.0
    sha: 2bc2ff7c20ae47c4a80f42a67d85d6889a5fad3bc2385aec6dc9b58795920c5b
  - name: gstreamer-sys
    version: 0.23.0
    sha: d9c9005b55dd2b1784645963c1ec409f9d420a56f6348d0ae69c2eaff584bcc3
  - name: gstreamer-video
    version: 0.23.0
    sha: 57332bca1ae7825a53fe57d993b63389f132d335aed691ac76f0ffe4304548e3
  - name: gstreamer-video-sys
    version: 0.23.0
    sha: 0f5c334d143384e8dc714af948c2e5d7d12cb588fdcfb56f3bf37c24daf350ef
  - name: gtk4
    version: 0.9.1
    sha: f4fe572bf318e5dbc6f5a2f8a25d853f1ae3f42768c0b08af6ca20a18f4057e1
  - name: gtk4-macros
    version: 0.9.1
    sha: e9e7b362c8fccd2712297903717d65d30defdab2b509bc9d209cbe5ffb9fabaf
  - name: gtk4-sys
    version: 0.9.0
    sha: 1114a207af8ada02cf4658a76692f4190f06f093380d5be07e3ca8b43aa7c666
  - name: hashbrown
    version: 0.14.5
    sha: e5274423e17b7c9fc20b6e7e208532f9b19825d82dfd615708b70edd83df41f1
  - name: heck
    version: 0.5.0
    sha: 2304e00983f87ffb38b55b444b5e3b60a884b5d30c0fca7d82fe33449bbe55ea
  - name: iana-time-zone
    version: 0.1.60
    sha: e7ffbb5a1b541ea2561f8c41c087286cc091e21e556a4f09a8f6cbf17b69b141
  - name: iana-time-zone-haiku
    version: 0.1.2
    sha: f31827a206f56af32e590ba56d5d2d085f558508192593743f16b2306495269f
  - name: indexmap
    version: 2.4.0
    sha: 93ead53efc7ea8ed3cfb0c79fc8023fbb782a5432b52830b6518941cebe6505c
  - name: itertools
    version: 0.13.0
    sha: 413ee7dfc52ee1a4949ceeb7dbc8a33f2d6c088194d9f922fb8318faf1f01186
  - name: js-sys
    version: 0.3.70
    sha: 1868808506b929d7b0cfa8f75951347aa71bb21144b7791bae35d9bccfcfe37a
  - name: libc
    version: 0.2.158
    sha: d8adc4bb1803a324070e64a98ae98f38934d91957a99cfb3a43dcbc01bc56439
  - name: lock_api
    version: 0.4.12
    sha: 07af8b9cdd281b7915f413fa73f29ebd5d55d0d3f0155584dade1ff18cea1b17
  - name: log
    version: 0.4.22
    sha: a7a70ba024b9dc04c27ea2f0c0548feb474ec5c54bba33a7f72f873a39d07b24
  - name: memchr
    version: 2.7.4
    sha: 78ca9ab1a0babb1e7d5695e3530886289c18cf2f87ec19a575a0abdce112e3a3
  - name: memoffset
    version: 0.9.1
    sha: 488016bfae457b036d996092f6cb448677611ce4449e970ceaf42695203f218a
  - name: muldiv
    version: 1.0.1
    sha: 956787520e75e9bd233246045d19f42fb73242759cc57fba9611d940ae96d4b0
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
    version: 1.19.0
    sha: 3fdb12b2476b595f9358c5161aa467c2438859caa136dec86c26fdd2efe17b92
  - name: option-operations
    version: 0.5.0
    sha: 7c26d27bb1aeab65138e4bf7666045169d1717febcc9ff870166be8348b223d0
  - name: pango
    version: 0.20.1
    sha: 5764e5a174a5a0ec054fe5962ce6d4fc7052e2d0dcc23bbc77202b40a4a403d3
  - name: pango-sys
    version: 0.20.1
    sha: fd317e1de76b14b3d3efe05518c08b360327f1ab7fec150473a89ffcad4b072d
  - name: parking
    version: 2.2.0
    sha: bb813b8af86854136c6922af0598d719255ecb2179515e6e7730d468f05c9cae
  - name: parking_lot
    version: 0.12.3
    sha: f1bf18183cf54e8d6059647fc3063646a1801cf30896933ec2311622cc4b9a27
  - name: parking_lot_core
    version: 0.9.10
    sha: 1e401f977ab385c9e4e3ab30627d6f26d00e2c73eef317493c4ec6d468726cf8
  - name: paste
    version: 1.0.15
    sha: 57c0d7b74b563b49d38dae00a0c37d4d6de9b432382b2892f0574ddcae73fd0a
  - name: pin-project-lite
    version: 0.2.14
    sha: bda66fc9667c18cb2758a2ac84d1167245054bcf85d5d1aaa6923f45801bdd02
  - name: pin-utils
    version: 0.1.0
    sha: 8b870d8c151b6f2fb93e84a13146138f05d02ed11c7e7c54f8826aaaf7c9f184
  - name: pkg-config
    version: 0.3.30
    sha: d231b230927b5e4ad203db57bbcbee2802f6bce620b1e4a9024a07d94e2907ec
  - name: proc-macro-crate
    version: 3.2.0
    sha: 8ecf48c7ca261d60b74ab1a7b20da18bede46776b2e55535cb958eb595c5fa7b
  - name: proc-macro2
    version: 1.0.86
    sha: 5e719e8df665df0d1c8fbfd238015744736151d4445ec0836b8e628aae103b77
  - name: quote
    version: 1.0.37
    sha: b5b9d34b8991d19d98081b46eacdd8eb58c6f2b201139f7c5f643cc155a633af
  - name: redox_syscall
    version: 0.5.3
    sha: 2a908a6e00f1fdd0dfd9c0eb08ce85126f6d8bbda50017e74bc4a4b7d4a926a4
  - name: rustc_version
    version: 0.4.0
    sha: bfa0f585226d2e68097d4f95d113b15b83a82e819ab25717ec0590d9584ef366
  - name: scopeguard
    version: 1.2.0
    sha: 94143f37725109f92c262ed2cf5e59bce7498c01bcc1502d7b9afe439a4e9f49
  - name: semver
    version: 1.0.23
    sha: 61697e0a1c7e512e84a621326239844a24d8207b4669b41bc18b32ea5cbf988b
  - name: serde
    version: 1.0.209
    sha: 99fce0ffe7310761ca6bf9faf5115afbc19688edd00171d81b1bb1b116c63e09
  - name: serde_derive
    version: 1.0.209
    sha: a5831b979fd7b5439637af1752d535ff49f4860c0f341d1baeb6faf0f4242170
  - name: serde_spanned
    version: 0.6.7
    sha: eb5b1b31579f3811bf615c144393417496f152e12ac8b7663bf664f4a815306d
  - name: shlex
    version: 1.3.0
    sha: 0fda2ff0d084019ba4d7c6f371c95d8fd75ce3524c3cb8fb653a3023f6323e64
  - name: slab
    version: 0.4.9
    sha: 8f92a496fb766b417c996b9c5e57daf2f7ad3b0bebe1ccfca4856390e3d3bb67
  - name: smallvec
    version: 1.13.2
    sha: 3c5e1a9a646d36c3599cd173a41282daf47c44583ad367b8e6837255952e5c67
  - name: syn
    version: 2.0.76
    sha: 578e081a14e0cefc3279b0472138c513f37b41a08d5a3cca9b6e4e8ceb6cd525
  - name: system-deps
    version: 7.0.2
    sha: 070a0a5e7da2d24be457809c4b3baa57a835fd2829ad8b86f9a049052fe71031
  - name: target-lexicon
    version: 0.12.16
    sha: 61c41af27dd6d1e27b1b16b489db798443478cef1f06a660c96db617ba5de3b1
  - name: thiserror
    version: 1.0.63
    sha: c0342370b38b6a11b6cc11d6a805569958d54cfa061a29969c3b5ce2ea405724
  - name: thiserror-impl
    version: 1.0.63
    sha: a4558b58466b9ad7ca0f102865eccc95938dca1a74a856f2b57b6629050da261
  - name: toml
    version: 0.8.19
    sha: a1ed1f98e3fdc28d6d910e6737ae6ab1a93bf1985935a1193e68f93eeb68d24e
  - name: toml_datetime
    version: 0.6.8
    sha: 0dd7358ecb8fc2f8d014bf86f6f638ce72ba252a2c3a2572f2a795f1d23efb41
  - name: toml_edit
    version: 0.22.20
    sha: 583c44c02ad26b0c3f3066fe629275e50627026c51ac2e595cca4c230ce1ce1d
  - name: unicode-ident
    version: 1.0.12
    sha: 3354b9ac3fae1ff6755cb6db53683adb661634f67557942dea4facebec0fee4b
  - name: version-compare
    version: 0.2.0
    sha: 852e951cb7832cb45cb1169900d19760cfa39b82bc0ea9c0e5a14ae88411c98b
  - name: wasm-bindgen
    version: 0.2.93
    sha: a82edfc16a6c469f5f44dc7b571814045d60404b55a0ee849f9bcfa2e63dd9b5
  - name: wasm-bindgen-backend
    version: 0.2.93
    sha: 9de396da306523044d3302746f1208fa71d7532227f15e347e2d93e4145dd77b
  - name: wasm-bindgen-macro
    version: 0.2.93
    sha: 585c4c91a46b072c92e908d99cb1dcdf95c5218eeb6f3bf1efa991ee7a68cccf
  - name: wasm-bindgen-macro-support
    version: 0.2.93
    sha: afc340c74d9005395cf9dd098506f7f44e38f2b4a21c6aaacf9a105ea5e1e836
  - name: wasm-bindgen-shared
    version: 0.2.93
    sha: c62a0a307cb4a311d3a07867860911ca130c3494e8c2719593806c08bc5d0484
  - name: windows-core
    version: 0.52.0
    sha: 33ab640c8d7e35bf8ba19b884ba838ceb4fba93a4e8c65a9059d08afcfc683d9
  - name: windows-sys
    version: 0.52.0
    sha: 282be5f36a8ce781fad8c8ae18fa3f9beff57ec1b52cb3de0789201425d9a33d
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
    version: 0.6.18
    sha: 68a9bda4691f099d435ad181000724da8e5899daa10713c2d432552b9ccd3a6f
