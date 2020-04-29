pam_subuid
==========

A PAM module that automatically populates `/etc/subuid` and `/etc/subgid` entries.

Subordinate user and group ID entries are required in order to use container
engines such as [Podman](https://podman.io/) in "rootless" mode.

Although recent versions of `useradd(8)` do populate `/etc/subuid` and
`/etc/subgid` when a user is added, systems that use an external identity
management system such as [FreeIPA](https://www.freeipa.org/) won't have any
entries in those files without 

Building
--------

You will need [Meson](https://mesonbuild.com/) installed.

```
$ meson setup . build --prefix=/

$ meson compile -C build

$ meson test -C build 
```

Installing
----------

```
$ meson install -C build
```

How you configure the module within your PAM stack depends. For example, on a
Debian system you can place the following at the end of
`/etc/pam.d/common-session`:

```
session optional pam_subuid.so
```

Then log in and check for newly created entries in `/etc/subuid` and
`/etc/subgid`.
