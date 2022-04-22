/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2012 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KWIN_FOCUS_CHAIN_H
#define KWIN_FOCUS_CHAIN_H
// KWin
#include <kwinglobals.h>
// Qt
#include <QHash>
#include <QObject>

namespace KWin
{
// forward declarations
class Window;
class Output;
class VirtualDesktop;

/**
 * @brief Singleton class to handle the various focus chains.
 *
 * A focus chain is a list of Clients containing information on which Client should be activated.
 *
 * Internally this FocusChain holds multiple independent chains. There is one chain of most recently
 * used Clients which is primarily used by TabBox to build up the list of Clients for navigation.
 * The chains are organized as a normal QList of Clients with the most recently used Client being the
 * last item of the list, that is a LIFO like structure.
 *
 * In addition there is one chain for each virtual desktop which is used to determine which Client
 * should get activated when the user switches to another virtual desktop.
 *
 * Furthermore this class contains various helper methods for the two different kind of chains.
 */
class FocusChain : public QObject
{
    Q_OBJECT
public:
    enum Change {
        MakeFirst,
        MakeLast,
        Update,
        MakeFirstMinimized = MakeFirst
    };
    ~FocusChain() override;
    /**
     * @brief Updates the position of the @p client according to the requested @p change in the
     * focus chain.
     *
     * This method affects both the most recently used focus chain and the per virtual desktop focus
     * chain.
     *
     * In case the client does no longer want to get focus, it is removed from all chains. In case
     * the client is on all virtual desktops it is ensured that it is present in each of the virtual
     * desktops focus chain. In case it's on exactly one virtual desktop it is ensured that it is only
     * in the focus chain for that virtual desktop.
     *
     * Depending on @p change the Client is inserted at different positions in the focus chain. In case
     * of @c MakeFirst it is moved to the first position of the chain, in case of
     * @c MakeLast it is moved to the last position of the chain. In all other cases it
     * depends on whether the @p client is the currently active Client. If it is the active Client it
     * becomes the first Client in the chain, otherwise it is inserted at the second position that is
     * directly after the currently active Client.
     *
     * @param client The Client which should be moved inside the chains.
     * @param change Where to move the Client
     * @return void
     */
    void update(Window *client, Change change);
    /**
     * @brief Moves @p client behind the @p reference Client in all focus chains.
     *
     * @param client The Client to move in the chains
     * @param reference The Client behind which the @p client should be moved
     * @return void
     */
    void moveAfterClient(Window *client, Window *reference);
    /**
     * @brief Finds the best Client to become the new active Client in the focus chain for the given
     * virtual @p desktop.
     *
     * In case that separate screen focus is used only Clients on the current screen are considered.
     * If no Client for activation is found @c null is returned.
     *
     * @param desktop The virtual desktop to look for a Client for activation
     * @return :X11Window *The Client which could be activated or @c null if there is none.
     */
    Window *getForActivation(VirtualDesktop *desktop) const;
    /**
     * @brief Finds the best Client to become the new active Client in the focus chain for the given
     * virtual @p desktop on the given @p screen.
     *
     * This method makes only sense to use if separate screen focus is used. If separate screen focus
     * is disabled the @p screen is ignored.
     * If no Client for activation is found @c null is returned.
     *
     * @param desktop The virtual desktop to look for a Client for activation
     * @param output The screen to constrain the search on with separate screen focus
     * @return :X11Window *The Client which could be activated or @c null if there is none.
     */
    Window *getForActivation(VirtualDesktop *desktop, Output *output) const;

    /**
     * @brief Checks whether the most recently used focus chain contains the given @p client.
     *
     * Does not consider the per-desktop focus chains.
     * @param client The Client to look for.
     * @return bool @c true if the most recently used focus chain contains @p client, @c false otherwise.
     */
    bool contains(Window *client) const;
    /**
     * @brief Checks whether the focus chain for the given @p desktop contains the given @p client.
     *
     * Does not consider the most recently used focus chain.
     *
     * @param client The Client to look for.
     * @param desktop The virtual desktop whose focus chain should be used
     * @return bool @c true if the focus chain for @p desktop contains @p client, @c false otherwise.
     */
    bool contains(Window *client, VirtualDesktop *desktop) const;
    /**
     * @brief Queries the most recently used focus chain for the next Client after the given
     * @p reference Client.
     *
     * The navigation wraps around the borders of the chain. That is if the @p reference Client is
     * the last item of the focus chain, the first Client will be returned.
     *
     * If the @p reference Client cannot be found in the focus chain, the first element of the focus
     * chain is returned.
     *
     * @param reference The start point in the focus chain to search
     * @return :X11Window *The relatively next Client in the most recently used chain.
     */
    Window *nextMostRecentlyUsed(Window *reference) const;
    /**
     * @brief Queries the focus chain for @p desktop for the next Client in relation to the given
     * @p reference Client.
     *
     * The method finds the first usable Client which is not the @p reference Client. If no Client
     * can be found @c null is returned
     *
     * @param reference The reference Client which should not be returned
     * @param desktop The virtual desktop whose focus chain should be used
     * @return :X11Window *The next usable Client or @c null if none can be found.
     */
    Window *nextForDesktop(Window *reference, VirtualDesktop *desktop) const;
    /**
     * @brief Returns the first Client in the most recently used focus chain. First Client in this
     * case means really the first Client in the chain and not the most recently used Client.
     *
     * @return :X11Window *The first Client in the most recently used chain.
     */
    Window *firstMostRecentlyUsed() const;

    bool isUsableFocusCandidate(Window *c, Window *prev) const;

public Q_SLOTS:
    /**
     * @brief Removes @p client from all focus chains.
     *
     * @param client The Client to remove from all focus chains.
     * @return void
     */
    void remove(KWin::Window *client);
    void setSeparateScreenFocus(bool enabled);
    void setActiveClient(KWin::Window *client);
    void setCurrentDesktop(VirtualDesktop *desktop);
    void addDesktop(VirtualDesktop *desktop);
    void removeDesktop(VirtualDesktop *desktop);

private:
    using Chain = QList<Window *>;
    /**
     * @brief Makes @p client the first Client in the given focus @p chain.
     *
     * This means the existing position of @p client is dropped and @p client is appended to the
     * @p chain which makes it the first item.
     *
     * @param client The Client to become the first in @p chain
     * @param chain The focus chain to operate on
     * @return void
     */
    void makeFirstInChain(Window *client, Chain &chain);
    /**
     * @brief Makes @p client the last Client in the given focus @p chain.
     *
     * This means the existing position of @p client is dropped and @p client is prepended to the
     * @p chain which makes it the last item.
     *
     * @param client The Client to become the last in @p chain
     * @param chain The focus chain to operate on
     * @return void
     */
    void makeLastInChain(Window *client, Chain &chain);
    void moveAfterClientInChain(Window *client, Window *reference, Chain &chain);
    void updateClientInChain(Window *client, Change change, Chain &chain);
    void insertClientIntoChain(Window *client, Chain &chain);
    Chain m_mostRecentlyUsed;
    QHash<VirtualDesktop *, Chain> m_desktopFocusChains;
    bool m_separateScreenFocus;
    Window *m_activeClient;
    VirtualDesktop *m_currentDesktop = nullptr;

    KWIN_SINGLETON_VARIABLE(FocusChain, s_manager)
};

inline bool FocusChain::contains(Window *client) const
{
    return m_mostRecentlyUsed.contains(client);
}

inline void FocusChain::setSeparateScreenFocus(bool enabled)
{
    m_separateScreenFocus = enabled;
}

inline void FocusChain::setActiveClient(Window *client)
{
    m_activeClient = client;
}

inline void FocusChain::setCurrentDesktop(VirtualDesktop *desktop)
{
    m_currentDesktop = desktop;
}

} // namespace

#endif // KWIN_FOCUS_CHAIN_H
