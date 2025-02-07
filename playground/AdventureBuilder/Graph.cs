using System;
using System.Collections;
using System.Drawing;
using System.IO;

namespace AdventureBuilder
{
  public delegate Graph GenGraph();
	/// <summary>
	/// Summary description for Graph.
	/// </summary>
	public class Graph : ISavable
	{
		public Graph()
		{
      m_roots = new ArrayList();
      m_leafs = new ArrayList();
      m_parent = null;
      m_type = Factory.ObjectType.Graph;
		}

    public Graph(BinaryReader reader) : this() 
    {
      m_type = Factory.ObjectType.Graph;
      ArrayList tmpnodes = new ArrayList();
      int count = reader.ReadInt32();
      for (int i = 0; i < count; ++i)
      {
        GraphNode node = (GraphNode)Factory.makeObject(reader);
        tmpnodes.Add(node);
        addUnconnectedNode(node);
      }

      count = reader.ReadInt32();
      for (int i = 0; i < count; ++i)
      {
        GraphEdge edge = (GraphEdge)Factory.makeObject(reader);
        int from = reader.ReadInt32();
        int to = reader.ReadInt32();
        edge.From = (GraphNode)tmpnodes[from];
        edge.To = (GraphNode)tmpnodes[to];
        addEdge(edge);
      }
    }

    public void save(BinaryWriter writer)
    {
      writer.Write((int)m_type);
      GraphFlattenVisitor gflv = new GraphFlattenVisitor();
      getVisited(gflv);
      writer.Write(gflv.Nodes.Count);
      foreach (GraphNode node in gflv.Nodes)
      {
        node.save(writer);
      }

      writer.Write(gflv.Edges.Count);
      foreach (GraphEdge edge in gflv.Edges)
      {
        edge.save(writer);
        int from = gflv.Nodes.IndexOf(edge.From);
        int to = gflv.Nodes.IndexOf(edge.To);
        writer.Write(from);
        writer.Write(to);
      }
    }

    //public virtual GraphNode newNode(Point p){
    //  return new GraphNode(p);
    //}

    public virtual GraphEdge newEdge(GraphNode from, GraphNode to){
      return new GraphEdge(from, to);
    }

    public void addUnconnectedNode(GraphNode node){
      m_roots.Add(node);
      m_leafs.Add(node);
    }

    public void addEdge(GraphEdge edge){
      //TODO
      //add from to roots if it was not in graph
      //add to to leafs if it was not in graph

      //build connections
      m_leafs.Remove(edge.From);
      //TODO
      if (!edge.IsLoop || edge.From.getRoot() != edge.To.getRoot())
        m_roots.Remove(edge.To);
      edge.From.Successors.Add(edge);
      edge.To.Predecessors.Add(edge);
    }

    public void connect(GraphNode from, GraphNode to){
      GraphEdge edge = newEdge(from, to);
      addEdge(edge);
    }

    virtual public object getVisited(GraphVisitor visitor){
      return visitor.visit(this);
    }

    virtual public object getVisited(AdventureGraphVisitor visitor){
      return visitor.visit(this);
    }

    public ArrayList Roots{
      get{return m_roots;}
    }

    public Graph Parent{
      set{m_parent = value;}
      get{return m_parent;}
    }

    private ArrayList m_roots;
    private ArrayList m_leafs;
    protected Factory.ObjectType m_type;
    private Graph m_parent;
	}
}
