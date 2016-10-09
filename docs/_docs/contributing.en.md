---
lang: en
layout: docs
title: Contributing
permalink: /docs/contributing.en/
---

Hi there! Interested in contributing to DarkFlow? We'd love your help. DarkFlow is an open source project and aims to be built by users like you.

## Where to get help or report a problem

* If you have a question about using DarkFlow, start a discussion by [opening an issue]({{ site.repository }}/issue/new).
* If you think you've found a bug in DarkFlow, [open an issue]({{ site.repository }}/issues/new).
* More resources are listed on our [Help page](/help.en/).

## Ways to contribute

Whether you're a developer, an amateur astronomer, or just a DarkFlow devotee, there are lots of ways to contribute. Here's a few ideas:

* [Install DarkFlow on your computer](/docs/installation.en/) and kick the tires. Does it work? Does it do what you'd expect? If not, [open an issue]({{ site.repository }}/issues/new) and let us know.
* Comment on some of the project's [open issues]({{ site.repository }}/issues). Have you experienced the same problem? Know a work around? Do you have a suggestion for how the feature could be better?
* Read through [the documentation](/docs/home.en/), and click the "improve this page" button, any time you see something confusing, or have a suggestion for something that could be improved.
* Browse through [DarkFlow issues]({{ site.repository }}/issues), and lend a hand answering questions. There's a good chance you've already experienced what another user is experiencing.
* Find [an open issue]({{ site.repository }}/issues) (especially [those labeled `help-wanted`]({{ site.repository }}/issues?q=is%3Aopen+is%3Aissue+label%3Ahelp-wanted)), and submit a proposed fix. If it's your first pull request, we promise we won't bite, and are glad to answer any questions.
* Help evaluate [open pull requests]({{ site.repository }}/pulls), by testing the changes locally and reviewing what's proposed.

## Submitting a pull request

### Pull requests generally

* The smaller the proposed change, the better. If you'd like to propose two unrelated changes, submit two pull requests.

* The more information, the better. Make judicious use of the pull request body. Describe what changes were made, why you made them, and what impact they will have for users.

* Pull request are easy and fun. If this is your first pull request, it may help to [understand GitHub Flow](https://guides.github.com/introduction/flow/).

* If you're submitting a code contribution, be sure to read the [code contributions](#code-contributions) section below.

### Submitting a pull request via github.com

Many small changes can be made entirely through the github.com web interface.

1. Navigate to the file within [`ploki/darkflow`]({{ site.repository }}) that you'd like to edit.
2. Click the pencil icon in the top right corner to edit the file
3. Make your proposed changes
4. Click "Propose file change"
5. Click "Create pull request"
6. Add a descriptive title and detailed description for your proposed change. The more information the better.
7. Click "Create pull request"

That's it! You'll be automatically subscribed to receive updates as others review your proposed change and provide feedback.

### Submitting a pull request via Git command line

1. Fork the project by clicking "Fork" in the top right corner of [`ploki/darkflow`]({{ site.repository }}).
2. Clone the repository locally `git clone https://github.com/<you-username>/darkflow`.
3. Create a new, descriptively named branch to contain your change ( `git checkout -b my-awesome-feature` ).
4. Hack away, compile, run and debug. Not necessarily in that order.
5. Make sure your modifications will not alter other's processes.
6. Push the branch up ( `git push origin my-awesome-feature` ).
7. Create a pull request by visiting `https://github.com/<your-username>/darkflow` and following the instructions at the top of the screen.

## Proposing updates to the documentation

We want the DarkFlow documentation to be the best it can be. We've open-sourced our docs and we welcome any pull requests if you find it lacking.

### How to submit changes

You can find the documentation for www.darkflow.org in the [site]({{ site.repository }}/tree/master/docs) directory. See the section above, [submitting a pull request](#submitting-a-pull-request) for information on how to propose a change.

One gotcha, all pull requests should be directed at the `master` branch (the default branch).

## Code Contributions

Interesting in submitting a pull request? Awesome. Read on. There's a few common gotchas that we'd love to help you avoid.

#### Documentation

Any time you propose a code change, you should also include updates to the documentation within the same pull request.

If your contribution changes any DarkFlow behavior, make sure to update the documentation. Documentation lives in the `docs/_docs` folder (spoiler alert: it's a Jekyll site!). If the docs are missing information, please feel free to add it in. Great docs make a great project. Include changes to the documentation within your pull request, and once merged, `www.darkflow.org` will be updated.

### Code contributions generally

* DarkFlow is developed with [Qt Creator](https://www.qt.io/ide/), the easiest way to get things done when it comes to Qt development. Just make sure the indentation is done the way Qt Creator wants it to be and your done.

* Dependencies may be added only if these are supported on all platforms DarkFlow runs on.

## A thank you

Thanks! Hacking on DarkFlow should be fun. If you find any of this hard to figure out, let us know so we can improve our process or documentation!
