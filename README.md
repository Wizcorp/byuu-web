# byuu-web

byuu is a multi-system emulator that aims to combine the accuracy of
[higan](https://byuu.org/higan) with the simplicity and performance of
[bsnes](https://byuu.org/bsnes).

This repository attempts to bring the goodness of byuu to the Web. We do
make significant changes to the repository due to performance restriction,
but try to stick to accuracy as much as possible. We also make sure
to keep uppdated with upstream, and as much as possible replicate the
same internal behaviors.

## Development

Git is used for the development of new releases, and represents a staging
environment. As byuu is rather mature, things should generally be quite stable.
However, bugs will exist, regressions will occur, so proceed at your own risk.

> Getting started

```shell
git clone [...] byuu-web
cd byuu-web
# We strongly recommend using the performance profile (turned off by default)
make -C higan target=web profile=performance
```

To achieve acceptable performances, byuu-web changes by default how the scheduler
is implemented and how each `Thread`'s main method behaves. This can be reverted
to test against upstream:

> Using the standard scheduler

```shell
make -C higan target=web profile=performance synchro=false
```

Once you have a build, start the HTTP development server.

> Serving the emulator

```shell
make -C higan serve
```

The emulator will be served at http://localhost:8000.

## Links

  - [Official website](https://byuu.org/byuu)
  - [Upstream git repository](https://github.com/byuu/byuu)
  - [Donations](https://patreon.com/byuu)

## Acknowledgements

We would like to give our sincere thanks to byuu for his contributions, help,
and support.
