#ifndef MAGEQUEST_CONNECTIONMAPPING_H
#define MAGEQUEST_CONNECTIONMAPPING_H

namespace magique
{
    struct ConnMapping final
    {
        Connection conn;
        entt::entity entity;

        static bool DeleteFunc(const ConnMapping& m1, const ConnMapping& m2) { return m1.conn == m2.conn; }
    };

    struct ConnNumberMapping final
    {
        std::array<Connection, MAGIQUE_MAX_PLAYERS - 1> conns{}; // Not invalid

        void addConnection(Connection conn)
        {
            for (auto& savedConn : conns)
            {
                if (savedConn == Connection::INVALID_CONNECTION)
                {
                    savedConn = conn;
                    return;
                }
            }
            LOG_ERROR("Too many connections");
        }

        void removeConnection(Connection conn)
        {
            for (auto& savedConn : conns)
            {
                if (savedConn == conn)
                {
                    savedConn = Connection::INVALID_CONNECTION;
                    return;
                }
            }
            LOG_ERROR("Connection could not be removed?!");
        }

        [[nodiscard]] int getNum(Connection conn) const
        {
            for (int i = 0; i < MAGIQUE_MAX_PLAYERS - 1; ++i)
            {
                if (conns[i] == conn)
                {
                    return i;
                }
            }
            return -1;
        }

        void clear()
        {
            for (auto& conn : conns)
            {
                conn = Connection::INVALID_CONNECTION;
            }
        }
    };

} // namespace magique
#endif //MAGEQUEST_CONNECTIONMAPPING_H
