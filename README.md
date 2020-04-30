pam_subuid
==========

A PAM module that automatically populates `/etc/subuid` and `/etc/subgid` when
a user logs in.

These files determine which user and group IDs are subordinate to a user.
Entries in these files are required for a user to be able to use container
engines such as [Podman](https://podman.io/) in "rootless" mode.

Where user accounts are defined by an identity management system such as
[FreeIPA](https://www.freeipa.org/) or [Active Directory Domain
Services](https://docs.microsoft.com/en-us/windows-server/identity/ad-ds/get-started/virtual-dc/active-directory-domain-services-overview)
won't have any entries in those files. You can use this module to avoid having
to create entries manually.

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
Debian machine, you can place the following at the end of
`/etc/pam.d/common-session`:

```
session optional pam_subuid.so
```

Then log in and check for newly created entries in `/etc/subuid` and
`/etc/subgid`.

Remarks
-------

⚠️ This module is not battle-hardened. Don't install it on your important
production cluster. But please try it out and send feedback (or pull requests!)

⚠️ I'm not an expert C programmer! I've used GNU extensions and probably
committed other sins that may turn the stomach of seasoned systems programmers!
🙇

`subuid` entries will be allocated from the range defined in `login.defs(5)` by
`SUB_UID_MIN` and `SUB_UID_MAX`. The number of entries is determined by
`SUB_UID_COUNT`.

`SUB_GID_MIN`, `SUB_GID_MAX` and `SUB_GID_COUNT` determine how `subgid` entires
are allocated.

You should adjust these settings to match your environment. For instance,
FreeIPA will (by default) allocate IDs between 200 000 and 2 000 000 000;
adjusting `SUB_[UG]ID_MAX` downward to 199 999 will prevent any possibility of
collision.

Allocations are handed out in the order that users log in, so they are unlikely
to be consistent between different machines.

If a user has an existing subuid or subgid allocation then they won't receive a
new one, even if the existing one is too small per `SUB_UID_COUNT` and
`SUB_GID_COUNT`.

The first field of `/etc/subuid` and `/etc/subgid` can be a user ID as well as
a user name. The module can't handle that yet.

Locking is performed to try to prevent two users logging in at the same time
from receiving the same allocation.

The module depends on the `usermod(8)` command to actaully modify the
subuid/subgid files.

Ongoing work in [shadow#154](https://github.com/shadow-maint/shadow/issues/154)
will evenutally render this project obsolete.

Acknowledgements
----------------

The code that searches for an unallocated subuid/subgid range was taken from
[shadow](https://github.com/shadow-maint/shadow).

We embed the [ctest](https://github.com/bvdberg/ctest) unit testing framework.
